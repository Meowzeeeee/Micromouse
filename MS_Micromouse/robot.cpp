#include "robot.h"
#include "maze.h"
#include "sensor.h"
#include "touch_sensor.h"
#include "rotating_ultrasonic_sensor.h"
#include <QDebug>

Robot::Robot(int startRow, int startCol, Direction startDir)
    : m_row(startRow), m_col(startCol), m_direction(startDir)
{
}

void Robot::attachSensor(std::shared_ptr<Sensor> sensor)
{
    m_sensors.push_back(sensor);
}

void Robot::resetVisited()
{
    m_visitedCount.clear();
}

void Robot::move(const Maze &maze)
{
    static int stepCounter = 0;
    stepCounter++;

    // --- Debug: wypisz pozycję, kierunek oraz ściany w bieżącej komórce ---
    qDebug() << "Robot at (" << m_row << "," << m_col << ") facing" << int(m_direction);
    qDebug() << "Maze walls at (" << m_row << "," << m_col << "):"
             << "Up="    << maze.hasWall(m_row, m_col, Direction::Up)
             << " Right=" << maze.hasWall(m_row, m_col, Direction::Right)
             << " Down="  << maze.hasWall(m_row, m_col, Direction::Down)
             << " Left="  << maze.hasWall(m_row, m_col, Direction::Left);

    // --- 1. Sprawdzenie czujnika dotykowego (przód) ---
    bool frontBlocked = false;
    for (auto &sensor : m_sensors) {
        if (auto *ts = dynamic_cast<TouchSensor*>(sensor.get())) {
            if (ts->detect(maze, *this)) {
                frontBlocked = true;
                qDebug() << "Touch sensor blocks front.";
                break;
            }
        }
    }

    // --- 2. Sprawdzenie sonaru bocznego (co 3 kroki) ---
    bool useSonar = (stepCounter % 3 == 0);
    bool leftBlocked = false, rightBlocked = false;
    for (auto &sensor : m_sensors) {
        if (auto *rotSonar = dynamic_cast<RotatingUltrasonicSensor*>(sensor.get())) {
            if (useSonar) {
                rotSonar->update(maze, *this);
                int distLeft  = rotSonar->getDistance(Direction::Left);
                int distRight = rotSonar->getDistance(Direction::Right);
                // Przyjmujemy: < 1 oznacza brak przejścia (0 = blokada, 1 = wolne pole)
                leftBlocked  = (distLeft < 1);
                rightBlocked = (distRight < 1);
                qDebug() << "Sonar side check: L=" << distLeft << " R=" << distRight
                         << " leftBlocked=" << leftBlocked << " rightBlocked=" << rightBlocked;
            }
            break; // zakładamy, że mamy jeden sensor rotacyjny
        }
    }

    // --- 3. Obliczamy kierunki oraz docelowe współrzędne dla przodu, lewo i prawo ---
    Direction leftDir, rightDir, frontDir;
    int forwardRow, forwardCol;
    int leftRow, leftCol;
    int rightRow, rightCol;

    switch (m_direction) {
    case Direction::Up:
        frontDir   = Direction::Up;
        leftDir    = Direction::Left;
        rightDir   = Direction::Right;
        forwardRow = m_row - 1; forwardCol = m_col;
        leftRow    = m_row;     leftCol   = m_col - 1;
        rightRow   = m_row;     rightCol  = m_col + 1;
        break;
    case Direction::Right:
        frontDir   = Direction::Right;
        leftDir    = Direction::Up;
        rightDir   = Direction::Down;
        forwardRow = m_row; forwardCol = m_col + 1;
        leftRow    = m_row - 1; leftCol = m_col;
        rightRow   = m_row + 1; rightCol = m_col;
        break;
    case Direction::Down:
        frontDir   = Direction::Down;
        leftDir    = Direction::Right;
        rightDir   = Direction::Left;
        forwardRow = m_row + 1; forwardCol = m_col;
        leftRow    = m_row; leftCol = m_col + 1;
        rightRow   = m_row; rightCol = m_col - 1;
        break;
    case Direction::Left:
        frontDir   = Direction::Left;
        leftDir    = Direction::Down;
        rightDir   = Direction::Up;
        forwardRow = m_row; forwardCol = m_col - 1;
        leftRow    = m_row + 1; leftCol = m_col;
        rightRow   = m_row - 1; rightCol = m_col;
        break;
    }

    // --- 4. Algorytm wall-follower: lewa ręka na ścianie ---
    // Kolejność: próba skrętu w lewo, jazda do przodu, próba skrętu w prawo, zawróć
    if (!maze.hasWall(m_row, m_col, leftDir) && !leftBlocked) {
        qDebug() << "Decision: Turn left";
        turnLeft();
        m_row = leftRow;
        m_col = leftCol;
        return;
    }
    if (!frontBlocked && !maze.hasWall(m_row, m_col, frontDir)) {
        qDebug() << "Decision: Move forward";
        m_row = forwardRow;
        m_col = forwardCol;
        return;
    }
    if (!maze.hasWall(m_row, m_col, rightDir) && !rightBlocked) {
        qDebug() << "Decision: Turn right";
        turnRight();
        m_row = rightRow;
        m_col = rightCol;
        return;
    }
    qDebug() << "Decision: Turn around (dead end)";
    turnRight();
    turnRight();
}

void Robot::turnLeft()
{
    switch (m_direction) {
    case Direction::Up:    m_direction = Direction::Left;  break;
    case Direction::Right: m_direction = Direction::Up;    break;
    case Direction::Down:  m_direction = Direction::Right; break;
    case Direction::Left:  m_direction = Direction::Down;  break;
    }
}

void Robot::turnRight()
{
    switch (m_direction) {
    case Direction::Up:    m_direction = Direction::Right; break;
    case Direction::Right: m_direction = Direction::Down;  break;
    case Direction::Down:  m_direction = Direction::Left;  break;
    case Direction::Left:  m_direction = Direction::Up;    break;
    }
}

bool Robot::isWallOnLeft(const Maze &maze)
{
    Direction leftDir;
    switch (m_direction) {
    case Direction::Up:    leftDir = Direction::Left;  break;
    case Direction::Right: leftDir = Direction::Up;    break;
    case Direction::Down:  leftDir = Direction::Right; break;
    case Direction::Left:  leftDir = Direction::Down;  break;
    }
    switch (leftDir) {
    case Direction::Up:    return maze.hasTopWall(m_row, m_col);
    case Direction::Right: return maze.hasRightWall(m_row, m_col);
    case Direction::Down:  return maze.hasBottomWall(m_row, m_col);
    case Direction::Left:  return maze.hasLeftWall(m_row, m_col);
    }
    return false;
}

bool Robot::isWallInFront(const Maze &maze)
{
    switch (m_direction) {
    case Direction::Up:    return maze.hasTopWall(m_row, m_col);
    case Direction::Right: return maze.hasRightWall(m_row, m_col);
    case Direction::Down:  return maze.hasBottomWall(m_row, m_col);
    case Direction::Left:  return maze.hasLeftWall(m_row, m_col);
    }
    return false;
}
