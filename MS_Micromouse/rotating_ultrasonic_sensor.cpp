#include "rotating_ultrasonic_sensor.h"
#include "maze.h"
#include "robot.h"

RotatingUltrasonicSensor::RotatingUltrasonicSensor(int range)
    : m_range(range), m_currentIndex(0)
{
    // Na starcie ustawiamy, że nie ma przeszkody w zasięgu (np. wartość m_range+1 oznacza brak wykrycia)
    m_measurements.fill(m_range + 1);
}

void RotatingUltrasonicSensor::update(const Maze &maze, const Robot &robot) {
    // Definiujemy kolejność skanowania względem aktualnego kierunku robota
    // Porządek: lewo, przód, prawo.
    std::array<Direction, 3> scanOrder;
    switch (robot.getDirection()) {
    case Direction::Up:
        scanOrder = { Direction::Left, Direction::Up, Direction::Right };
        break;
    case Direction::Right:
        scanOrder = { Direction::Up, Direction::Right, Direction::Down };
        break;
    case Direction::Down:
        scanOrder = { Direction::Right, Direction::Down, Direction::Left };
        break;
    case Direction::Left:
        scanOrder = { Direction::Down, Direction::Left, Direction::Up };
        break;
    }

    // Ustalamy bieżący kierunek skanowania
    Direction currentScanDir = scanOrder[m_currentIndex];

    // Wykonujemy pomiar: przechodzimy kolejne komórki aż do zasięgu m_range lub do napotkania przeszkody
    int distance = 0;
    int row = robot.getRow();
    int col = robot.getCol();
    bool obstacleFound = false;
    for (int i = 1; i <= m_range; ++i) {
        int checkRow = row;
        int checkCol = col;
        // Przesuwamy się o i pól w kierunku currentScanDir
        switch (currentScanDir) {
        case Direction::Up:    checkRow -= i; break;
        case Direction::Right: checkCol += i; break;
        case Direction::Down:  checkRow += i; break;
        case Direction::Left:  checkCol -= i; break;
        }

        // Jeśli poza labiryntem lub napotkamy ścianę, traktujemy to jako przeszkodę
        if (checkRow < 0 || checkRow >= maze.getHeight() ||
            checkCol < 0 || checkCol >= maze.getWidth() ||
            maze.hasWall(checkRow, checkCol, currentScanDir))
        {
            obstacleFound = true;
            distance = i; // dystans do przeszkody
            break;
        }
    }
    if (!obstacleFound) {
        distance = m_range + 1; // brak przeszkody w zasięgu
    }

    // Zapisujemy wynik dla aktualnego kierunku
    m_measurements[m_currentIndex] = distance;
    // Rotujemy – przy następnym update przeskanujemy kolejny kierunek
    m_currentIndex = (m_currentIndex + 1) % 3;
}

int RotatingUltrasonicSensor::getDistance(Direction relativeDir) const {
    // Mapowanie relatywnego kierunku na indeks:
    // Założenie: Left → 0, Front (Up) → 1, Right → 2.
    int idx = 1; // domyślnie front
    switch(relativeDir) {
    case Direction::Left:  idx = 0; break;
    case Direction::Up:    idx = 1; break;
    case Direction::Right: idx = 2; break;
    default: break;
    }
    return m_measurements[idx];
}
