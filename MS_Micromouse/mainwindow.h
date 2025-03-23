#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QMovie>  // do GIF-a
#include <memory>

#include "maze.h"
#include "robot.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMapSliderChanged(int value);
    void onSpeedDialChanged(int value);
    void onStartClicked();
    void onStopClicked();
    void onRestartClicked();
    void updateSimulation();

private:
    Ui::MainWindow *ui;

    std::unique_ptr<Maze> m_maze;
    std::unique_ptr<Robot> m_robot;
    QGraphicsScene *m_scene;

    QTimer m_timer;
    QElapsedTimer m_elapsed;

    bool m_isRunning = false;
    int m_moveCount = 0;

    // Koordynaty celu (środek labiryntu)
    int m_targetRow = 0;
    int m_targetCol = 0;

    // Limit ruchów (by uniknąć nieskończonej pętli)
    int m_maxMoves = 2000;

    // QMovie do animowanego GIF-a (spinner)
    QMovie *m_spinnerMovie = nullptr;

    void createMazeAndRobot();
    void resetUI();

    void drawMaze();
    void drawRobot();
};

#endif // MAINWINDOW_H
