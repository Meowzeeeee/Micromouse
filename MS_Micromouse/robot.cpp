#include "robot.h"
#include "maze.h"
#include "sensor.h"
#include "touch_sensor.h"
#include "ultrasonic_sensor.h"
#include <algorithm>
#include <tuple>
#include <climits>
#include <QDebug>


Robot::Robot(int startRow, int startCol)
    : m_row(startRow), m_col(startCol), m_direction(Direction::Right) // domyślnie ustawiamy kierunek na prawo
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

void Robot::move(const Maze& maze)
{
    // Zwiększ licznik odwiedzin bieżącej komórki
    m_visitedCount[{m_row, m_col}]++;
    qDebug() << "Robot at:" << m_row << m_col << "Visits:" << m_visitedCount[{m_row, m_col}];

    bool touchDetected = false;
    bool ultrasonicDetected = false;

    // Sprawdź dane z czujników
    for (const auto& sensor : m_sensors) {
        if (sensor->detect(maze, *this)) {
            if (dynamic_cast<TouchSensor*>(sensor.get())) {
                touchDetected = true;
                qDebug() << "Touch sensor detected obstacle.";
            }
            else if (dynamic_cast<UltrasonicSensor*>(sensor.get())) {
                ultrasonicDetected = true;
                qDebug() << "Ultrasonic sensor detected obstacle.";
            }
        }
    }

    // Jeśli dotykowy czujnik wykryje przeszkodę, skręć natychmiast w prawo.
    if (touchDetected) {
        qDebug() << "Decision: Touch detected, turning right.";
        turnRight();
        return;
    }

    // Wyznacz pomocnicze kierunki oraz współrzędne następców:
    Direction leftDir, rightDir;
    int leftRow, leftCol, rightRow, rightCol, forwardRow, forwardCol;
    switch (m_direction) {
    case Direction::Up:
        leftDir = Direction::Left;  rightDir = Direction::Right;
        leftRow = m_row;  leftCol = m_col - 1;
        rightRow = m_row; rightCol = m_col + 1;
        forwardRow = m_row - 1; forwardCol = m_col;
        break;
    case Direction::Right:
        leftDir = Direction::Up;  rightDir = Direction::Down;
        leftRow = m_row - 1; leftCol = m_col;
        rightRow = m_row + 1; rightCol = m_col;
        forwardRow = m_row; forwardCol = m_col + 1;
        break;
    case Direction::Down:
        leftDir = Direction::Right; rightDir = Direction::Left;
        leftRow = m_row;  leftCol = m_col + 1;
        rightRow = m_row; rightCol = m_col - 1;
        forwardRow = m_row + 1; forwardCol = m_col;
        break;
    case Direction::Left:
        leftDir = Direction::Down; rightDir = Direction::Up;
        leftRow = m_row + 1; leftCol = m_col;
        rightRow = m_row - 1; rightCol = m_col;
        forwardRow = m_row; forwardCol = m_col - 1;
        break;
    }

    // 1. Jeśli lewo jest wolne i nigdy tam nie byliśmy – wykonaj skręt w lewo.
    if (!maze.hasWall(m_row, m_col, leftDir) && m_visitedCount[{leftRow, leftCol}] == 0) {
        qDebug() << "Decision: Left is free and unvisited. Turning left.";
        turnLeft();
        m_row = leftRow;
        m_col = leftCol;
        return;
    }

    // 2. Jeśli przód jest wolny i nie wykrywa przeszkody ultradźwiękowej – jedź prosto.
    if (!maze.hasWall(m_row, m_col, m_direction) && !ultrasonicDetected && m_visitedCount[{forwardRow, forwardCol}] == 0) {
        qDebug() << "Decision: Forward is free and unvisited. Moving forward.";
        m_row = forwardRow;
        m_col = forwardCol;
        return;
    }

    // 3. Jeśli prawo jest wolne i nie odwiedzone – wykonaj skręt w prawo.
    if (!maze.hasWall(m_row, m_col, rightDir) && m_visitedCount[{rightRow, rightCol}] == 0) {
        qDebug() << "Decision: Right is free and unvisited. Turning right.";
        turnRight();
        m_row = rightRow;
        m_col = rightCol;
        return;
    }

    // 4. Jeśli wszystkie dostępne kierunki były odwiedzone lub czujnik ultradźwiękowy sygnalizuje przeszkodę,
    //    wybieramy opcję z najmniejszą liczbą odwiedzin, ale z karą dla ruchu "prosto", jeśli ultradźwięk jest aktywny.
    std::vector<std::tuple<Direction, int, int, int>> options;
    auto score = [this, ultrasonicDetected](int r, int c, bool isForward) {
        int visits = m_visitedCount[{r, c}];
        int penalty = (isForward && ultrasonicDetected) ? 1000 : 0;
        return visits + penalty;
    };

    if (!maze.hasWall(m_row, m_col, leftDir))
        options.emplace_back(leftDir, leftRow, leftCol, score(leftRow, leftCol, false));
    if (!maze.hasWall(m_row, m_col, m_direction))
        options.emplace_back(m_direction, forwardRow, forwardCol, score(forwardRow, forwardCol, true));
    if (!maze.hasWall(m_row, m_col, rightDir))
        options.emplace_back(rightDir, rightRow, rightCol, score(rightRow, rightCol, false));

    if (options.empty()) {
        // Jeśli robot jest całkowicie zablokowany, zawróć.
        qDebug() << "No available moves, turning around.";
        turnRight(); turnRight();
        return;
    }

    std::sort(options.begin(), options.end(),
              [](const auto& a, const auto& b) {
                  return std::get<3>(a) < std::get<3>(b);
              });
    Direction bestDir = std::get<0>(options.front());
    int targetRow = std::get<1>(options.front());
    int targetCol = std::get<2>(options.front());
    int bestWeight = std::get<3>(options.front());
    qDebug() << "Best option: Dir:" << int(bestDir)
             << "Target:" << targetRow << targetCol
             << "Weight:" << bestWeight;

    // Dopasuj kierunek
    if (bestDir != m_direction) {
        int diff = (int(bestDir) + 4 - int(m_direction)) % 4;
        if (diff == 1) {
            qDebug() << "Turning right to match best option.";
            turnRight();
        } else if (diff == 3) {
            qDebug() << "Turning left to match best option.";
            turnLeft();
        } else if (diff == 2) {
            qDebug() << "Turning around.";
            turnRight();
            turnRight();
        }
    }

    qDebug() << "Moving to:" << targetRow << targetCol;
    m_row = targetRow;
    m_col = targetCol;
}

void Robot::turnLeft()
{
    // Implementacja skrętu w lewo
    switch (m_direction) {
    case Direction::Up:    m_direction = Direction::Left;  break;
    case Direction::Right: m_direction = Direction::Up;    break;
    case Direction::Down:  m_direction = Direction::Right; break;
    case Direction::Left:  m_direction = Direction::Down;  break;
    }
}

void Robot::turnRight()
{
    // Implementacja skrętu w prawo
    switch (m_direction) {
    case Direction::Up:    m_direction = Direction::Right; break;
    case Direction::Right: m_direction = Direction::Down;  break;
    case Direction::Down:  m_direction = Direction::Left;  break;
    case Direction::Left:  m_direction = Direction::Up;    break;
    }
}

bool Robot::isWallOnLeft(const Maze& maze)
{
    // Ustal, która strona jest "lewa" w zależności od aktualnego kierunku robota
    Direction leftDir;
    switch (m_direction) {
    case Direction::Up:    leftDir = Direction::Left;  break;
    case Direction::Right: leftDir = Direction::Up;    break;
    case Direction::Down:  leftDir = Direction::Right; break;
    case Direction::Left:  leftDir = Direction::Down;  break;
    }

    // Sprawdz obecność ściany na tej stronie
    switch (leftDir) {
    case Direction::Up:    return maze.hasTopWall(m_row, m_col);
    case Direction::Right: return maze.hasRightWall(m_row, m_col);
    case Direction::Down:  return maze.hasBottomWall(m_row, m_col);
    case Direction::Left:  return maze.hasLeftWall(m_row, m_col);
    }
    return false;
}

bool Robot::isWallInFront(const Maze& maze)
{
    switch (m_direction) {
    case Direction::Up:    return maze.hasTopWall(m_row, m_col);
    case Direction::Right: return maze.hasRightWall(m_row, m_col);
    case Direction::Down:  return maze.hasBottomWall(m_row, m_col);
    case Direction::Left:  return maze.hasLeftWall(m_row, m_col);
    }
    return false;
}
