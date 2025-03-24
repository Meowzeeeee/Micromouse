#ifndef ROBOT_H
#define ROBOT_H

#include <vector>
#include <memory>
#include <map>
#include <utility>
#include "maze.h"  // Klasa Maze
#include "direction.h"

// Deklaracja klasy Sensor (ogólna)
class Sensor;

class Robot
{
public:
    // Konstruktor – teraz przyjmuje też startowy kierunek (domyślnie Down)
    Robot(int startRow, int startCol, Direction startDir = Direction::Down);

    // Metoda wykonująca jeden krok ruchu robota
    void move(const Maze &maze);

    int getRow() const { return m_row; }
    int getCol() const { return m_col; }
    Direction getDirection() const { return m_direction; }

    // Metoda dołączania czujników
    void attachSensor(std::shared_ptr<Sensor> sensor);

    // Reset pamięci odwiedzonych pól
    void resetVisited();

private:
    int m_row;
    int m_col;
    Direction m_direction;

    // Wektor wskaźników do czujników (TouchSensor, RotatingUltrasonicSensor, itp.)
    std::vector<std::shared_ptr<Sensor>> m_sensors;
    std::map<std::pair<int, int>, int> m_visitedCount;

    // Metody pomocnicze do zmiany kierunku
    void turnLeft();
    void turnRight();

    // Opcjonalnie:
    bool isWallOnLeft(const Maze &maze);
    bool isWallInFront(const Maze &maze);
};

#endif // ROBOT_H
