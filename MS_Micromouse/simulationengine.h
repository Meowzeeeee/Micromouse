#ifndef SIMULATIONENGINE_H
#define SIMULATIONENGINE_H

#include <QObject>
#include <memory>
#include <QTimer>
#include <QElapsedTimer>
#include "maze.h"
#include "robot.h"

class SimulationEngine : public QObject {
    Q_OBJECT

public:
    explicit SimulationEngine(QObject *parent = nullptr);

    void setAlgorithm(Robot::MovementAlgorithm algo);
    void setMapSize(int size);

public slots:
    void start(int speedMultiplier);
    void stop();

signals:
    void mazeChanged(const Maze &maze);
    void robotMoved(int row, int col);
    void simulationFinished(double elapsedSec, int moves, bool reachedCenter);

private slots:
    void onTimeout();

private:
    std::unique_ptr<Maze>  m_maze;
    std::unique_ptr<Robot> m_robot;

    QTimer        m_timer;
    QElapsedTimer m_elapsed;
    int           m_moveCount = 0;
    int           m_maxMoves  = 2000;
    int           m_mapSize   = 10;
    Robot::MovementAlgorithm m_algo = Robot::MovementAlgorithm::Sensors;
};

#endif // SIMULATIONENGINE_H
