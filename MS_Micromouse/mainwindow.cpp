#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "touch_sensor.h"
#include "ultrasonic_sensor.h"

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

    // Wyniki (Results, Time, Moves) niewidoczne na start
    ui->resultsTitleLabel->setVisible(false);
    ui->timeLabel->setVisible(false);
    ui->movesLabel->setVisible(false);

    // 1. Inicjalizujemy QMovie z zasobów (np. ":/spinner.gif")
    //   (Upewnij się, że w spinner.qrc jest <file>spinner.gif</file> z prefix="/")
    m_spinnerMovie = new QMovie(":/spinner.gif", QByteArray(), this);
    // Ewentualnie skaluj
    m_spinnerMovie->setScaledSize(QSize(100, 66));
    ui->loadingLabel->setMovie(m_spinnerMovie);
    m_spinnerMovie->start();
    // Początkowo etykieta spinnera jest niewidoczna
    ui->loadingLabel->setVisible(false);

    // Teksty początkowe
    ui->mapSizeValueLabel->setText(QString("%1x%1").arg(ui->mapSlider->value()));
    ui->speedValueLabel->setText(QString("%1x").arg(ui->speedDial->value()));

    // Podłączenie sygnałów
    connect(ui->mapSlider, &QSlider::valueChanged, this, &MainWindow::onMapSliderChanged);
    connect(ui->speedDial, &QDial::valueChanged,   this, &MainWindow::onSpeedDialChanged);

    connect(ui->startButton,   &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(ui->stopButton,    &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(ui->restartButton, &QPushButton::clicked, this, &MainWindow::onRestartClicked);

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

    m_maze  = std::make_unique<Maze>(mapSize, mapSize);
    m_robot = std::make_unique<Robot>(0, 0);

    // RESETUJEMY PAMIĘĆ ODWIEDZONYCH PÓL
    m_robot->resetVisited();

    // Czujnik dotykowy – wykrywa ścianę bezpośrednio przed robotem
    auto touchSensor = std::make_shared<TouchSensor>();
    m_robot->attachSensor(touchSensor);

    // Czujniki ultradźwiękowe – "widzą" na odległość w różnych kierunkach
    m_robot->attachSensor(std::make_shared<UltrasonicSensor>(2, Direction::Up));     // Przód
    m_robot->attachSensor(std::make_shared<UltrasonicSensor>(2, Direction::Left));   // Lewo
    m_robot->attachSensor(std::make_shared<UltrasonicSensor>(2, Direction::Right));  // Prawo

    // Koordynaty celu (środek labiryntu)
    m_targetRow = mapSize / 2;
    m_targetCol = mapSize / 2;

    m_moveCount = 0;

    drawMaze();
    drawRobot();
}


// --------------------------------------------------------------------------
// RESET UI
// --------------------------------------------------------------------------
void MainWindow::resetUI()
{
    ui->resultsTitleLabel->setVisible(false);
    ui->timeLabel->setVisible(false);
    ui->movesLabel->setVisible(false);

    ui->timeLabel->setText("Time: 0 s");
    ui->movesLabel->setText("Moves: 0");
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

        // Pokazujemy spinner
        ui->loadingLabel->setVisible(true);

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

        // Chowamy spinner
        ui->loadingLabel->setVisible(false);

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
// UPDATE SIMULATION
// --------------------------------------------------------------------------
void MainWindow::updateSimulation()
{
    m_robot->move(*m_maze);
    m_moveCount++;

    drawMaze();
    drawRobot();

    // Sprawdzenie, czy robot dotarł do celu
    if (m_robot->getRow() == m_targetRow && m_robot->getCol() == m_targetCol) {
        m_timer.stop();
        m_isRunning = false;

        // Chowamy spinner
        ui->loadingLabel->setVisible(false);

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

    // Limit m_maxMoves
    if (m_moveCount >= m_maxMoves) {
        m_timer.stop();
        m_isRunning = false;

        ui->loadingLabel->setVisible(false);

        ui->startButton->setEnabled(true);

        qint64 elapsedMs = m_elapsed.elapsed();
        double elapsedSec = elapsedMs / 1000.0;

        ui->timeLabel->setText(
            QString("Time: %1 s (not reached center)").arg(elapsedSec, 0, 'f', 2));
        ui->movesLabel->setText(QString("Moves: %1").arg(m_moveCount));

        ui->resultsTitleLabel->setVisible(true);
        ui->timeLabel->setVisible(true);
        ui->movesLabel->setVisible(true);
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
    QBrush targetBrush(Qt::green);

    int width = m_maze->getWidth();
    int height = m_maze->getHeight();

    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int x = c * cellSize;
            int y = r * cellSize;

            // Pole docelowe na zielono
            if (r == m_targetRow && c == m_targetCol) {
                m_scene->addRect(x, y, cellSize, cellSize, Qt::NoPen, targetBrush);
            }

            // Ściany
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
