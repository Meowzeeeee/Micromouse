#include "touch_sensor.h"
#include "maze.h"
#include "robot.h"

bool TouchSensor::detect(const Maze &maze, const Robot &robot)
{
    // 1. Ustalamy, która komórka jest "z przodu" (nextRow, nextCol)
    int row = robot.getRow();
    int col = robot.getCol();

    int nextRow = row;
    int nextCol = col;

    switch (robot.getDirection()) {
    case Direction::Up:
        nextRow = row - 1;
        break;
    case Direction::Right:
        nextCol = col + 1;
        break;
    case Direction::Down:
        nextRow = row + 1;
        break;
    case Direction::Left:
        nextCol = col - 1;
        break;
    }

    // 2. Jeśli nextRow/nextCol jest poza labiryntem -> traktujemy to jak ścianę
    if (nextRow < 0 || nextRow >= maze.getHeight() ||
        nextCol < 0 || nextCol >= maze.getWidth())
    {
        return true; // "dotykamy" ściany granicznej
    }

    // 3. Sprawdź, czy Maze ma ścianę pomiędzy (row,col) a (nextRow,nextCol)
    //    w kierunku, w którym robot się porusza.
    return maze.hasWall(row, col, robot.getDirection());
}
