#include "simulationengine.h"

SimulationEngine::SimulationEngine(QObject *parent)
    : QObject(parent)
    , m_timer(this)
{
    connect(&m_timer, &QTimer::timeout,
            this, &SimulationEngine::onTimeout);
}

void SimulationEngine::setAlgorithm(Robot::MovementAlgorithm algo) {
    m_algo = algo;
    if (m_robot) m_robot->setAlgorithm(algo);
}

void SimulationEngine::setMapSize(int size) {
    m_mapSize = size;
}

void SimulationEngine::setSpeed(int speedMultiplier) {
    m_speedMultiplier = speedMultiplier;
    // jeśli timer aktywny, zmień interwał od razu
    if (m_timer.isActive()) {
        const int baseInterval = 200;
        m_timer.start(baseInterval / m_speedMultiplier);
    }
}

void SimulationEngine::start(int speedMultiplier) {
    // Jeżeli był pauzowany → wznowienie
    if (m_paused) {
        const int baseInterval = 200;
        m_timer.start(baseInterval / m_speedMultiplier);
        m_paused = false;
        return;
    }

    // Nowa symulacja:
    m_speedMultiplier = speedMultiplier;
    m_paused = false;

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

    // Start timera wg prędkości
    const int baseInterval = 200;
    m_timer.start(baseInterval / m_speedMultiplier);
}

void SimulationEngine::stop() {
    // Pauza (bez emitowania wyników)
    if (!m_timer.isActive()) return;
    m_timer.stop();
    m_paused = true;
}

void SimulationEngine::restart(int speedMultiplier) {
    // Restart symulacji od razu (ignoruj pauzę)
    m_paused = false;
    start(speedMultiplier);
}

void SimulationEngine::onTimeout() {
    // Krok robota + sygnał do GUI
    m_robot->move(*m_maze);
    ++m_moveCount;
    emit robotMoved(m_robot->getRow(), m_robot->getCol());

    // Jeśli cel osiągnięty lub limit ⇒ zakończ
    if ((m_robot->getRow() == m_mapSize/2 &&
         m_robot->getCol() == m_mapSize/2) ||
        m_moveCount >= m_maxMoves)
    {
        finish();
    }
}

void SimulationEngine::finish() {
    m_timer.stop();
    m_paused = false;

    double elapsed = m_elapsed.elapsed() / 1000.0;
    bool reached = (m_robot->getRow() == m_mapSize/2 &&
                    m_robot->getCol() == m_mapSize/2);
    emit simulationFinished(elapsed, m_moveCount, reached);
}
