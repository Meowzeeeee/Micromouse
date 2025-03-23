#include "robot.h"
#include "maze.h"

Robot::Robot(int startRow, int startCol)
    : m_row(startRow), m_col(startCol), m_direction(Direction::Right) // domyślnie ustawiamy kierunek na prawo
{
}

void Robot::attachSensor(std::shared_ptr<Sensor> sensor)
{
    m_sensors.push_back(sensor);
}

void Robot::move(const Maze& maze)
{
    // Przykładowa logika "trzymaj się lewej ściany":
    // 1. Jeśli po lewej nie ma ściany -> skręć w lewo.
    // 2. Jeśli po lewej jest ściana, ale przed sobą brak ściany -> jedź do przodu.
    // 3. Jeśli przed sobą jest ściana -> skręć w prawo.

    if (!isWallOnLeft(maze)) {
        turnLeft();
    } else if (isWallInFront(maze)) {
        turnRight();
    }

    // Obliczenie nowej pozycji na podstawie kierunku
    int nextRow = m_row;
    int nextCol = m_col;
    switch (m_direction) {
    case Direction::Up:    nextRow--; break;
    case Direction::Right: nextCol++; break;
    case Direction::Down:  nextRow++; break;
    case Direction::Left:  nextCol--; break;
    }

    // Sprawdzamy, czy nie ma ściany na kierunku ruchu (bazując na metodach z Maze)
    bool blocked = false;
    if (m_direction == Direction::Up && maze.hasTopWall(m_row, m_col))
        blocked = true;
    else if (m_direction == Direction::Right && maze.hasRightWall(m_row, m_col))
        blocked = true;
    else if (m_direction == Direction::Down && maze.hasBottomWall(m_row, m_col))
        blocked = true;
    else if (m_direction == Direction::Left && maze.hasLeftWall(m_row, m_col))
        blocked = true;

    if (!blocked) {
        m_row = nextRow;
        m_col = nextCol;
    }

    // Tu dodać wywołania dla czujników
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
