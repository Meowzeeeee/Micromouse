#include "simulationengine.h"

SimulationEngine::SimulationEngine(QObject *parent)
    : QObject(parent), m_timer(this)
{
    connect(&m_timer, &QTimer::timeout,
            this, &SimulationEngine::onTimeout);
}

void SimulationEngine::setAlgorithm(Robot::MovementAlgorithm algo) {
    m_algo = algo;
}

void SimulationEngine::setMapSize(int size) {
    m_mapSize = size;
}

void SimulationEngine::start(int speedMultiplier) {
    m_maze  = std::make_unique<Maze>(m_mapSize, m_mapSize);
    int center = m_mapSize / 2;
    m_robot = std::make_unique<Robot>(
        0, 0,
        Direction::Down,
        m_algo,
        center, center
        );
    m_moveCount = 0;
    m_elapsed.restart();
    emit mazeChanged(*m_maze);
    int baseInterval = 200;
    m_timer.start(baseInterval / speedMultiplier);
}

void SimulationEngine::stop() {
    if (!m_timer.isActive()) return;
    m_timer.stop();
    double elapsed = m_elapsed.elapsed() / 1000.0;
    bool reached = (m_robot->getRow() == m_mapSize/2 &&
                    m_robot->getCol() == m_mapSize/2);
    emit simulationFinished(elapsed, m_moveCount, reached);
}

void SimulationEngine::onTimeout() {
    m_robot->move(*m_maze);
    ++m_moveCount;
    emit robotMoved(m_robot->getRow(), m_robot->getCol());
    if (m_robot->getRow() == m_mapSize/2 &&
        m_robot->getCol() == m_mapSize/2) {
        stop();
    } else if (m_moveCount >= m_maxMoves) {
        stop();
    }
}
