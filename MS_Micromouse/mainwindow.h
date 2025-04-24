#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QMovie>
#include <memory>

#include "maze.h"               // potrzebne do Maze
#include "simulationengine.h"   // SimulationEngine
#include "robot.h"              // Robot::MovementAlgorithm

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QGraphicsScene;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onStartClicked();
    void onStopClicked();
    void onRestartClicked();
    void onMapSliderChanged(int value);
    void onSpeedDialChanged(int value);
    void onAlgorithmChanged(int id);
    void drawMaze(const Maze &maze);
    void drawRobot(int row, int col);
    void onSimulationFinished(double elapsedSec, int moves, bool reached);

private:
    Ui::MainWindow *ui;

    // silnik symulacji
    std::unique_ptr<SimulationEngine> m_engine;

    // scena do rysowania
    QGraphicsScene *m_scene = nullptr;

    // spinner (GIF)
    QMovie *m_spinnerMovie = nullptr;

    // grupa przycisków algorytmu
    QButtonGroup *m_algoGroup = nullptr;

    // pamiętamy stan labiryntu, by móc przerysować przed robotem
    Maze m_currentMaze{8, 8};
};

#endif // MAINWINDOW_H
