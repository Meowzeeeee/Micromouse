#include "ultrasonic_sensor.h"
#include "maze.h"
#include "robot.h"
#include "robot.h"  // Upewnij się, że enum Direction jest widoczny

UltrasonicSensor::UltrasonicSensor(int range)
    : m_range(range)
{
}

bool UltrasonicSensor::detect(const Maze &maze, const Robot &robot) {
    int row = robot.getRow();
    int col = robot.getCol();

    // Sprawdzamy kolejne komórki w kierunku, w którym zwrócony jest robot.
    for (int i = 1; i <= m_range; ++i) {
        int checkRow = row;
        int checkCol = col;
        switch(robot.getDirection()) {
        case Direction::Up:    checkRow = row - i; break;
        case Direction::Right: checkCol = col + i; break;
        case Direction::Down:  checkRow = row + i; break;
        case Direction::Left:  checkCol = col - i; break;
        }
        // Jeśli wychodzimy poza granice labiryntu, przerywamy.
        if (checkRow < 0 || checkRow >= maze.getHeight() ||
            checkCol < 0 || checkCol >= maze.getWidth()) {
            break;
        }
        // Prosta logika: jeśli w danej komórce w kierunku ruchu znajduje się ściana, czujnik wykrywa przeszkodę.
        if (robot.getDirection() == Direction::Up && maze.hasTopWall(checkRow, checkCol))
            return true;
        else if (robot.getDirection() == Direction::Right && maze.hasRightWall(checkRow, checkCol))
            return true;
        else if (robot.getDirection() == Direction::Down && maze.hasBottomWall(checkRow, checkCol))
            return true;
        else if (robot.getDirection() == Direction::Left && maze.hasLeftWall(checkRow, checkCol))
            return true;
    }
    return false;
}
