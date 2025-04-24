#ifndef ROBOT_H
#define ROBOT_H

#include <vector>
#include <memory>
#include <utility>
#include "maze.h"
#include "direction.h"

class TouchSensor;
class RotatingUltrasonicSensor;

class Robot
{
public:
    enum class MovementAlgorithm { Sensors = 0, FloodFill = 1, AStar = 2 };

    Robot(int startRow,
          int startCol,
          Direction startDir = Direction::Down,
          MovementAlgorithm algo = MovementAlgorithm::Sensors,
          int goalRow = 0,
          int goalCol = 0);
    ~Robot();  // destruktor w CPP, widzi pełne defini­cje sensorów

    void move(const Maze &maze);

    int getRow() const { return m_row; }
    int getCol() const { return m_col; }
    Direction getDirection() const { return m_direction; }

    MovementAlgorithm getAlgorithm() const { return m_algorithm; }
    void setAlgorithm(MovementAlgorithm algo) { m_algorithm = algo; }

private:
    int m_row;
    int m_col;
    Direction m_direction;

    // kompozycja czujników — robot je tworzy i niszczy
    std::unique_ptr<TouchSensor>              m_touchSensor;
    std::unique_ptr<RotatingUltrasonicSensor> m_rotSonar;

    MovementAlgorithm m_algorithm;
    int m_goalRow;
    int m_goalCol;

    // dla A*
    std::vector<std::pair<int,int>> m_path;
    size_t m_pathIndex = 0;

    // trzy strategie ruchu:
    void moveWithSensors(const Maze &maze);
    void moveFloodFill   (const Maze &maze);
    void moveAStar       (const Maze &maze);

    void turnLeft();
    void turnRight();
};

#endif // ROBOT_H
