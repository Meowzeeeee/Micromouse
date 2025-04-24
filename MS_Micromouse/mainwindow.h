#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QMovie>
#include <memory>

#include "maze.h"               // definicja Maze
#include "simulationengine.h"   // klasa SimulationEngine
#include "robot.h"              // enum MovementAlgorithm

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
    void onStartClicked();              // uruchomienie lub wznowienie symulacji
    void onStopClicked();               // pauza symulacji
    void onRestartClicked();            // restart labiryntu
    void onMapSliderChanged(int value); // zmiana rozmiaru mapy
    void onSpeedDialChanged(int value); // zmiana prędkości animacji
    void onAlgorithmChanged(int id);    // przełączanie algorytmu
    void drawMaze(const Maze &maze);    // rysowanie labiryntu
    void drawRobot(int row, int col);   // rysowanie pozycji robota
    void onSimulationFinished(double elapsedSec, int moves, bool reached); // wyniki

private:
    Ui::MainWindow            *ui;
    std::unique_ptr<SimulationEngine> m_engine;    // silnik symulacji
    QGraphicsScene            *m_scene        = nullptr;  // scena do rysunku
    QMovie                    *m_spinnerMovie = nullptr;  // animowany GIF
    QButtonGroup              *m_algoGroup    = nullptr;  // grupa radiobuttonów
    Maze                        m_currentMaze{8, 8};      // pamiętany stan labiryntu
};

#endif // MAINWINDOW_H
