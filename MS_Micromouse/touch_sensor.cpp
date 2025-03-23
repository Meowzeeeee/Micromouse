#include "touch_sensor.h"
#include "maze.h"
#include "robot.h"

bool TouchSensor::detect(const Maze &maze, const Robot &robot) {
    int row = robot.getRow();
    int col = robot.getCol();
    switch(robot.getDirection()) {
    case Direction::Up:
        return maze.hasTopWall(row, col);
    case Direction::Right:
        return maze.hasRightWall(row, col);
    case Direction::Down:
        return maze.hasBottomWall(row, col);
    case Direction::Left:
        return maze.hasLeftWall(row, col);
    }
    return false;
}
