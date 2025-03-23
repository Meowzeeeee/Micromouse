#ifndef ROBOT_H
#define ROBOT_H

#include <vector>
#include <memory>
//#include <set>
#include <map>

#include <utility>
#include "maze.h"  // Upewnij się, że masz dołączoną klasę Maze
#include "direction.h"


// Deklaracja klasy Sensor (na razie pusta)
class Sensor;

class Robot
{
public:
    // Konstruktor przyjmujący początkową pozycję (wiersz i kolumna)
    Robot(int startRow, int startCol);

    // Metoda, która wykonuje ruch robota na podstawie obecnego labiryntu.
    void move(const Maze& maze);

    int getRow() const { return m_row; }
    int getCol() const { return m_col; }
    Direction getDirection() const { return m_direction; }

    // Metoda pozwalająca podłączyć czujnik
    void attachSensor(std::shared_ptr<Sensor> sensor);

    //Metoda czyszczenia zapamietanych komorek
    void resetVisited();



private:
    int m_row;
    int m_col;
    Direction m_direction;

    // Wektor wskaźników do czujników – na razie puste
    std::vector<std::shared_ptr<Sensor>> m_sensors;

    //Wektor, zapisanie pozycji
    //std::set<std::pair<int, int>> m_visited;
    std::map<std::pair<int, int>, int> m_visitedCount;


    // Proste metody pomocnicze do zmiany kierunku
    void turnLeft();
    void turnRight();

    // Metody sprawdzające, czy w labiryncie znajduje się ściana w określonym kierunku
    bool isWallOnLeft(const Maze& maze);
    bool isWallInFront(const Maze& maze);
};

#endif // ROBOT_H
