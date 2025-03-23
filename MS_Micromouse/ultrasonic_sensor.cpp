#include "ultrasonic_sensor.h"
#include "maze.h"
#include "robot.h"

UltrasonicSensor::UltrasonicSensor(int range, Direction direction)
    : m_range(range), m_direction(direction)
{
}

bool UltrasonicSensor::detect(const Maze &maze, const Robot &robot) {
    int row = robot.getRow();
    int col = robot.getCol();

    // Sprawdź 3 kierunki: lewo, przód, prawo
    std::vector<Direction> directions;

    switch (robot.getDirection()) {
    case Direction::Up:
        directions = {Direction::Left, Direction::Up, Direction::Right};
        break;
    case Direction::Right:
        directions = {Direction::Up, Direction::Right, Direction::Down};
        break;
    case Direction::Down:
        directions = {Direction::Right, Direction::Down, Direction::Left};
        break;
    case Direction::Left:
        directions = {Direction::Down, Direction::Left, Direction::Up};
        break;
    }

    for (Direction dir : directions) {
        int checkRow = row;
        int checkCol = col;

        for (int i = 1; i <= m_range; ++i) {
            switch (dir) {
            case Direction::Up:    checkRow--; break;
            case Direction::Right: checkCol++; break;
            case Direction::Down:  checkRow++; break;
            case Direction::Left:  checkCol--; break;
            }

            if (checkRow < 0 || checkRow >= maze.getHeight() ||
                checkCol < 0 || checkCol >= maze.getWidth()) {
                break;
            }

            if (!maze.hasWall(checkRow, checkCol, dir)) {
                return true;  // Widzimy wolne miejsce w tym kierunku
            }
        }
    }

    return false; // Wszystko zablokowane
}
