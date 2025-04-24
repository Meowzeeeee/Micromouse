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

    void setAlgorithm(Robot::MovementAlgorithm algo);  // zmiana algorytmu
    void setMapSize(int size);                         // rozmiar labiryntu
    void setSpeed(int speedMultiplier);                // prędkość symulacji

public slots:
    void start(int speedMultiplier);   // start / wznowienie symulacji
    void stop();                       // pauza symulacji
    void restart(int speedMultiplier); // restart symulacji od nowa

signals:
    void mazeChanged(const Maze &maze);                       // nowy labirynt
    void robotMoved(int row, int col);                        // krok robota
    void simulationFinished(double elapsedSec, int moves, bool reachedCenter);

private slots:
    void onTimeout();  // krok timera

private:
    void finish();     // zakończenie (robot w centrum lub limit kroków)

    std::unique_ptr<Maze>  m_maze;
    std::unique_ptr<Robot> m_robot;
    QTimer        m_timer;
    QElapsedTimer m_elapsed;
    int           m_moveCount       = 0;
    int           m_maxMoves        = 2000;
    int           m_mapSize         = 10;
    int           m_speedMultiplier = 1;
    bool          m_paused          = false;
    Robot::MovementAlgorithm m_algo = Robot::MovementAlgorithm::Sensors;
};

#endif // SIMULATIONENGINE_H
