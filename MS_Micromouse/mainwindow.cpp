#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsScene>
#include <QPen>
#include <QBrush>
#include <QButtonGroup>
#include <QSlider>
#include <QDial>
#include <QPushButton>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_engine(std::make_unique<SimulationEngine>(this))
{
    ui->setupUi(this);

    // 1) Scena i view
    m_scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(m_scene);

    // 2) Spinner
    m_spinnerMovie = new QMovie(":/spinner.gif", QByteArray(), this);
    m_spinnerMovie->setScaledSize(QSize(100,66));
    ui->loadingLabel->setMovie(m_spinnerMovie);
    m_spinnerMovie->start();
    ui->loadingLabel->setVisible(false);

    // 3) RadioButtons – algorytm
    m_algoGroup = new QButtonGroup(this);
    m_algoGroup->addButton(ui->rbSensors,   static_cast<int>(Robot::MovementAlgorithm::Sensors));
    m_algoGroup->addButton(ui->rbFloodFill, static_cast<int>(Robot::MovementAlgorithm::FloodFill));
    m_algoGroup->addButton(ui->rbAStar,     static_cast<int>(Robot::MovementAlgorithm::AStar));
    connect(m_algoGroup,
            static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::idClicked),
            this,
            &MainWindow::onAlgorithmChanged);

    // 4) GUI → slots
    connect(ui->startButton,   &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(ui->stopButton,    &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(ui->restartButton, &QPushButton::clicked, this, &MainWindow::onRestartClicked);
    connect(ui->mapSlider,     &QSlider::valueChanged, this, &MainWindow::onMapSliderChanged);
    connect(ui->speedDial,     &QDial::valueChanged,   this, &MainWindow::onSpeedDialChanged);

    // 5) Silnik → GUI
    connect(m_engine.get(), &SimulationEngine::mazeChanged,
            this, &MainWindow::drawMaze);
    connect(m_engine.get(), &SimulationEngine::robotMoved,
            this, &MainWindow::drawRobot);
    connect(m_engine.get(), &SimulationEngine::simulationFinished,
            this, &MainWindow::onSimulationFinished);

    // 6) Etykiety startowe
    ui->mapSizeValueLabel->setText(QString("%1x%1").arg(ui->mapSlider->value()));
    ui->speedValueLabel->setText(QString("%1x").arg(ui->speedDial->value()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Zmień algorytm
void MainWindow::onAlgorithmChanged(int id)
{
    m_engine->setAlgorithm(static_cast<Robot::MovementAlgorithm>(id));
}

// Rozmiar mapy
void MainWindow::onMapSliderChanged(int value)
{
    ui->mapSizeValueLabel->setText(QString("%1x%1").arg(value));
}

// Prędkośc
void MainWindow::onSpeedDialChanged(int value)
{
    ui->speedValueLabel->setText(QString("%1x").arg(value));
}

// Start
void MainWindow::onStartClicked()
{
    ui->resultsTitleLabel->setVisible(false);
    ui->timeLabel->setVisible(false);
    ui->movesLabel->setVisible(false);
    ui->loadingLabel->setVisible(true);

    m_engine->setMapSize(ui->mapSlider->value());
    m_engine->start(ui->speedDial->value());
}

// Stop
void MainWindow::onStopClicked()
{
    m_engine->stop();
}

// Restart
void MainWindow::onRestartClicked()
{
    m_engine->stop();
    ui->loadingLabel->setVisible(false);
    drawMaze(Maze(ui->mapSlider->value(), ui->mapSlider->value()));
}

// Rysuj labirynt
void MainWindow::drawMaze(const Maze &maze)
{
    m_currentMaze = maze;
    m_scene->clear();

    int cellSize = 40;
    QPen pen(Qt::black, 2);
    QBrush targetBrush(Qt::green);

    int W = maze.getWidth(), H = maze.getHeight();
    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            int x = c * cellSize, y = r * cellSize;
            if (r == H/2 && c == W/2)
                m_scene->addRect(x, y, cellSize, cellSize, Qt::NoPen, targetBrush);
            if (maze.hasTopWall(r,c))
                m_scene->addLine(x, y, x+cellSize, y, pen);
            if (maze.hasRightWall(r,c))
                m_scene->addLine(x+cellSize, y, x+cellSize, y+cellSize, pen);
            if (maze.hasBottomWall(r,c))
                m_scene->addLine(x, y+cellSize, x+cellSize, y+cellSize, pen);
            if (maze.hasLeftWall(r,c))
                m_scene->addLine(x, y, x, y+cellSize, pen);
        }
    }
}

// Rysuj robota
void MainWindow::drawRobot(int row, int col)
{
    // przerysuj najpierw cały labirynt
    drawMaze(m_currentMaze);

    // potem robot
    int cellSize = 40, m = 6;
    QPen pen(Qt::red);
    QBrush brush(Qt::red);
    int x = col*cellSize + m, y = row*cellSize + m;
    m_scene->addEllipse(x, y, cellSize-2*m, cellSize-2*m, pen, brush);
}

// Koniec symulacji
void MainWindow::onSimulationFinished(double elapsedSec, int moves, bool /*reached*/)
{
    ui->loadingLabel->setVisible(false);
    ui->timeLabel ->setText(QString("Time: %1 s").arg(elapsedSec, 0, 'f', 2));
    ui->movesLabel->setText(QString("Moves: %1").arg(moves));
    ui->resultsTitleLabel->setVisible(true);
    ui->timeLabel->setVisible(true);
    ui->movesLabel->setVisible(true);
}
