#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Tworzymy scenę i przypinamy do QGraphicsView
    m_scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(m_scene);

    // Ukrywamy etykiety wyników
    ui->resultsTitleLabel->setVisible(false);
    ui->timeLabel->setVisible(false);
    ui->movesLabel->setVisible(false);

    // Ustawianie tekstów początkowych
    ui->mapSizeValueLabel->setText(QString("%1x%1").arg(ui->mapSlider->value()));
    ui->speedValueLabel->setText(QString("%1x").arg(ui->speedDial->value()));

    // Podłączenie sygnałów
    connect(ui->mapSlider, &QSlider::valueChanged, this, &MainWindow::onMapSliderChanged);
    connect(ui->speedDial, &QDial::valueChanged,   this, &MainWindow::onSpeedDialChanged);

    connect(ui->startButton,   &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(ui->stopButton,    &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(ui->restartButton, &QPushButton::clicked, this, &MainWindow::onRestartClicked);

    // Timer co X ms -> updateSimulation()
    connect(&m_timer, &QTimer::timeout, this, &MainWindow::updateSimulation);

    // Domyślna inicjalizacja
    createMazeAndRobot();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --------------------------------------------------------------------------
// SUWAK MAPY
// --------------------------------------------------------------------------
void MainWindow::onMapSliderChanged(int value)
{
    ui->mapSizeValueLabel->setText(QString("%1x%1").arg(value));
}

// --------------------------------------------------------------------------
// DIAL PRĘDKOŚCI
// --------------------------------------------------------------------------
void MainWindow::onSpeedDialChanged(int value)
{
    ui->speedValueLabel->setText(QString("%1x").arg(value));

    if (m_isRunning) {
        int baseInterval = 200;
        int newInterval = baseInterval / value;
        m_timer.start(newInterval);
    }
}

// --------------------------------------------------------------------------
// CREATE MAZE & ROBOT
// --------------------------------------------------------------------------
void MainWindow::createMazeAndRobot()
{
    int mapSize = ui->mapSlider->value();

    // Tworzymy labirynt
    m_maze  = std::make_unique<Maze>(mapSize, mapSize);

    // Robot startuje w (0,0)
    m_robot = std::make_unique<Robot>(0, 0);

    // Ustawiamy cel jako środek labiryntu
    // (dla parzystego mapSize można wybrać (mapSize/2 - 1, mapSize/2 - 1))
    m_targetRow = mapSize / 2;
    m_targetCol = mapSize / 2;

    // Reset liczników
    m_moveCount = 0;
    ui->progressBar->setValue(0);

    // Rysujemy od razu labirynt i robota
    drawMaze();
    drawRobot();
}

// --------------------------------------------------------------------------
// RESET UI
// --------------------------------------------------------------------------
void MainWindow::resetUI()
{
    // Ukrywamy etykiety wyników
    ui->resultsTitleLabel->setVisible(false);
    ui->timeLabel->setVisible(false);
    ui->movesLabel->setVisible(false);

    ui->timeLabel->setText("Time: 0 s");
    ui->movesLabel->setText("Moves: 0");

    ui->progressBar->setValue(0);
    m_moveCount = 0;
}

// --------------------------------------------------------------------------
// START
// --------------------------------------------------------------------------
void MainWindow::onStartClicked()
{
    if (!m_isRunning) {
        m_isRunning = true;

        createMazeAndRobot();
        resetUI();

        m_elapsed.restart();

        int baseInterval = 200;
        int newInterval = baseInterval / ui->speedDial->value();
        m_timer.start(newInterval);

        ui->startButton->setEnabled(false);
    }
}

// --------------------------------------------------------------------------
// STOP
// --------------------------------------------------------------------------
void MainWindow::onStopClicked()
{
    if (m_isRunning) {
        m_isRunning = false;
        m_timer.stop();

        ui->startButton->setEnabled(true);

        // Wyniki
        qint64 elapsedMs = m_elapsed.elapsed();
        double elapsedSec = elapsedMs / 1000.0;
        ui->timeLabel->setText(QString("Time: %1 s").arg(elapsedSec, 0, 'f', 2));
        ui->movesLabel->setText(QString("Moves: %1").arg(m_moveCount));

        ui->resultsTitleLabel->setVisible(true);
        ui->timeLabel->setVisible(true);
        ui->movesLabel->setVisible(true);
    }
}

// --------------------------------------------------------------------------
// RESTART
// --------------------------------------------------------------------------
void MainWindow::onRestartClicked()
{
    if (m_isRunning) {
        m_isRunning = false;
        m_timer.stop();
        ui->startButton->setEnabled(true);
    }

    createMazeAndRobot();
    resetUI();
    drawMaze();
    drawRobot();
}

// --------------------------------------------------------------------------
// UPDATE SIMULATION (pętla timera)
// --------------------------------------------------------------------------
void MainWindow::updateSimulation()
{
    // 1. Robot robi ruch
    m_robot->move(*m_maze);
    m_moveCount++;

    // 2. Rysujemy labirynt i robota
    drawMaze();
    drawRobot();

    // 3. Postęp — np. liczymy procent względem m_maxMoves
    int progress = (100 * m_moveCount) / m_maxMoves;
    ui->progressBar->setValue(progress);

    // 4a. Robot dotarł do celu (środek)
    if (m_robot->getRow() == m_targetRow && m_robot->getCol() == m_targetCol) {
        m_timer.stop();
        m_isRunning = false;

        ui->startButton->setEnabled(true);

        qint64 elapsedMs = m_elapsed.elapsed();
        double elapsedSec = elapsedMs / 1000.0;
        ui->timeLabel->setText(QString("Time: %1 s").arg(elapsedSec, 0, 'f', 2));
        ui->movesLabel->setText(QString("Moves: %1").arg(m_moveCount));

        ui->resultsTitleLabel->setVisible(true);
        ui->timeLabel->setVisible(true);
        ui->movesLabel->setVisible(true);

        return;
    }

    // 4b. Robot nie doszedł do środka, ale przekroczono m_maxMoves
    if (m_moveCount >= m_maxMoves) {
        m_timer.stop();
        m_isRunning = false;

        ui->startButton->setEnabled(true);

        qint64 elapsedMs = m_elapsed.elapsed();
        double elapsedSec = elapsedMs / 1000.0;
        ui->timeLabel->setText(
            QString("Time: %1 s (not reached center)")
                .arg(elapsedSec, 0, 'f', 2));
        ui->movesLabel->setText(QString("Moves: %1").arg(m_moveCount));

        ui->resultsTitleLabel->setVisible(true);
        ui->timeLabel->setVisible(true);
        ui->movesLabel->setVisible(true);

        // QMessageBox::information(this, "Micromouse", "Robot didn't reach center in time.");
        return;
    }
}

// --------------------------------------------------------------------------
// RYSOWANIE LABIRYNTU
// --------------------------------------------------------------------------
void MainWindow::drawMaze()
{
    if (!m_maze) return;

    m_scene->clear();

    int cellSize = 40;
    QPen pen(Qt::black, 2);
    // Możemy dodać brush do celu
    QBrush targetBrush(Qt::green);  // zielony dla pola docelowego

    int width = m_maze->getWidth();
    int height = m_maze->getHeight();

    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int x = c * cellSize;
            int y = r * cellSize;

            // Jeśli to komórka docelowa, malujemy zielone tło
            if (r == m_targetRow && c == m_targetCol) {
                m_scene->addRect(x, y, cellSize, cellSize,
                                 Qt::NoPen, targetBrush);
            }

            // Rysowanie ścian
            if (m_maze->hasTopWall(r, c)) {
                m_scene->addLine(x, y, x + cellSize, y, pen);
            }
            if (m_maze->hasRightWall(r, c)) {
                m_scene->addLine(x + cellSize, y, x + cellSize, y + cellSize, pen);
            }
            if (m_maze->hasBottomWall(r, c)) {
                m_scene->addLine(x, y + cellSize, x + cellSize, y + cellSize, pen);
            }
            if (m_maze->hasLeftWall(r, c)) {
                m_scene->addLine(x, y, x, y + cellSize, pen);
            }
        }
    }
}

// --------------------------------------------------------------------------
// RYSOWANIE ROBOTA
// --------------------------------------------------------------------------
void MainWindow::drawRobot()
{
    if (!m_robot) return;

    int cellSize = 40;
    int row = m_robot->getRow();
    int col = m_robot->getCol();

    int x = col * cellSize;
    int y = row * cellSize;

    int margin = 6;
    QPen pen(Qt::red);
    QBrush brush(Qt::red);

    m_scene->addEllipse(x + margin, y + margin,
                        cellSize - 2*margin, cellSize - 2*margin,
                        pen, brush);
}
