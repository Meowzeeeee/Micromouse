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

    // 1) Ustawienie QGraphicsScene dla graphicsView
    m_scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(m_scene);

    // 2) Konfiguracja spinnera
    m_spinnerMovie = new QMovie(":/spinner.gif", QByteArray(), this);
    m_spinnerMovie->setScaledSize(QSize(100,66));
    ui->loadingLabel->setMovie(m_spinnerMovie);
    m_spinnerMovie->start();
    ui->loadingLabel->setVisible(false);

    // 3) Grupa radiobuttonów: wybór algorytmu
    m_algoGroup = new QButtonGroup(this);
    m_algoGroup->addButton(ui->rbSensors,   static_cast<int>(Robot::MovementAlgorithm::Sensors));
    m_algoGroup->addButton(ui->rbFloodFill, static_cast<int>(Robot::MovementAlgorithm::FloodFill));
    m_algoGroup->addButton(ui->rbAStar,     static_cast<int>(Robot::MovementAlgorithm::AStar));
    connect(m_algoGroup,
            static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::idClicked),
            this,
            &MainWindow::onAlgorithmChanged);

    // 4) Podłączenie sygnałów GUI do slotów
    connect(ui->startButton,   &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(ui->stopButton,    &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(ui->restartButton, &QPushButton::clicked, this, &MainWindow::onRestartClicked);
    connect(ui->mapSlider,     &QSlider::valueChanged, this, &MainWindow::onMapSliderChanged);
    connect(ui->speedDial,     &QDial::valueChanged,   this, &MainWindow::onSpeedDialChanged);

    // 5) Podłączenie sygnałów silnika do GUI
    connect(m_engine.get(), &SimulationEngine::mazeChanged,
            this, &MainWindow::drawMaze);
    connect(m_engine.get(), &SimulationEngine::robotMoved,
            this, &MainWindow::drawRobot);
    connect(m_engine.get(), &SimulationEngine::simulationFinished,
            this, &MainWindow::onSimulationFinished);

    // 6) Ustawienie początkowych wartości etykiet
    ui->mapSizeValueLabel->setText(QString("%1x%1").arg(ui->mapSlider->value()));
    ui->speedValueLabel->setText(QString("%1x").arg(ui->speedDial->value()));

    // Przy starcie symulacji przycisk Stop wyłączony
    ui->stopButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onAlgorithmChanged(int id)
{
    // Przekaż nowy algorytm do silnika
    m_engine->setAlgorithm(static_cast<Robot::MovementAlgorithm>(id));
}

void MainWindow::onMapSliderChanged(int value)
{
    // Wyświetl nowy rozmiar mapy
    ui->mapSizeValueLabel->setText(QString("%1x%1").arg(value));
}

void MainWindow::onSpeedDialChanged(int value)
{
    // Wyświetl nową prędkość i przekaż do silnika
    ui->speedValueLabel->setText(QString("%1x").arg(value));
    m_engine->setSpeed(value);
}

void MainWindow::onStartClicked()
{
    // Start lub wznowienie symulacji:
    // 1) Wyłącz Start, włącz Stop
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);

    // 2) Ukryj poprzednie wyniki, pokaż spinner
    ui->resultsTitleLabel->setVisible(false);
    ui->timeLabel->setVisible(false);
    ui->movesLabel->setVisible(false);
    ui->loadingLabel->setVisible(true);

    // 3) Ustaw rozmiar mapy i startuj/wznów
    m_engine->setMapSize(ui->mapSlider->value());
    m_engine->start(ui->speedDial->value());
}

void MainWindow::onStopClicked()
{
    // Pauza symulacji:
    // 1) Pauzuj silnik
    m_engine->stop();
    // 2) Włącz Start, wyłącz Stop
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void MainWindow::onRestartClicked()
{
    // Restart symulacji od nowa:
    // 1) Pauzuj ewentualnie trwającą symulację
    m_engine->stop();
    // 2) Wyłącz oba przyciski chwilowo
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(false);

    // 3) Ukryj poprzednie wyniki, pokaż spinner
    ui->resultsTitleLabel->setVisible(false);
    ui->timeLabel->setVisible(false);
    ui->movesLabel->setVisible(false);
    ui->loadingLabel->setVisible(true);

    // 4) Wygeneruj nową mapę i robota, a potem od razu restartuj
    m_engine->setMapSize(ui->mapSlider->value());
    m_engine->restart(ui->speedDial->value());

    // 5) Po restarcie: wyłącz Start, włącz Stop
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
}

void MainWindow::drawMaze(const Maze &maze)
{
    // Zapamiętaj stan i wyczyść scenę
    m_currentMaze = maze;
    m_scene->clear();

    // Rysuj komórki i ściany
    int cellSize = 40;
    QPen pen(Qt::black, 2);
    QBrush targetBrush(Qt::green);
    int W = maze.getWidth(), H = maze.getHeight();

    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            int x = c * cellSize, y = r * cellSize;
            // Pole celu na zielono
            if (r == H/2 && c == W/2)
                m_scene->addRect(x, y, cellSize, cellSize, Qt::NoPen, targetBrush);
            // Ściany każdej krawędzi
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
    // Przerysuj labirynt, potem dodaj robota
    drawMaze(m_currentMaze);
    int cellSize = 40, margin = 6;
    QPen pen(Qt::red);
    QBrush brush(Qt::red);
    int x = col*cellSize + margin;
    int y = row*cellSize + margin;
    m_scene->addEllipse(x, y, cellSize-2*margin, cellSize-2*margin, pen, brush);
}

// Koniec symulacji – pokaz wyniki tylko jeśli reached == true
void MainWindow::onSimulationFinished(double elapsedSec, int moves, bool reached)
{
    // 1) Zawsze wyłącz spinner
    ui->loadingLabel->setVisible(false);

    // 2) Wyłącz przyciski Stop i włącz Start
    ui->stopButton->setEnabled(false);
    ui->startButton->setEnabled(true);

    // 3) Jeżeli robot dotarł do celu — pokaż wyniki
    if (reached) {
        ui->timeLabel ->setText(QString("Time: %1 s")
                                   .arg(elapsedSec, 0, 'f', 2));
        ui->movesLabel->setText(QString("Moves: %1")
                                    .arg(moves));

        ui->resultsTitleLabel->setVisible(true);
        ui->timeLabel->setVisible(true);
        ui->movesLabel->setVisible(true);
    }
    // w przeciwnym razie nie pokazujemy etykiet wyników
}
