#include <iostream>
#include <memory>
#include "maze.h"
#include "robot.h"
#include "ultrasonic_sensor.h"
#include "touch_sensor.h"

// Funkcja rysująca labirynt w konsoli jako ASCII art.
// Jeśli w danej komórce znajduje się robot, zamiast podłogi rysujemy literę 'R'.
void printMaze(const Maze &maze, const Robot &robot) {
    int width = maze.getWidth();
    int height = maze.getHeight();

    // Rysujemy górną granicę labiryntu
    for (int c = 0; c < width; c++) {
        std::cout << " _";
    }
    std::cout << std::endl;

    // Dla każdej komórki wypisujemy ściany i robota
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            // Lewa granica (dla pierwszej kolumny)
            if (c == 0)
                std::cout << "|";

            // Jeśli robot znajduje się w tej komórce, rysujemy 'R'
            if (r == robot.getRow() && c == robot.getCol()) {
                std::cout << "R";
            } else {
                // Rysujemy dolną ścianę, jeśli jest ustawiona; w przeciwnym razie spację
                std::cout << (maze.hasBottomWall(r, c) ? "_" : " ");
            }
            // Prawa ściana – jeśli jest obecna, rysujemy '|', inaczej spację
            std::cout << (maze.hasRightWall(r, c) ? "|" : " ");
        }
        std::cout << std::endl;
    }
}

int main() {
    // Ustawiamy rozmiar labiryntu na nieparzystą wartość, aby łatwo wyznaczyć środek.
    int mazeSize = 11;
    Maze maze(mazeSize, mazeSize);
    Robot robot(0, 0);  // Robot startuje z lewego górnego rogu (0,0)

    // Wyznaczamy cel jako środek labiryntu (dla 11x11 będzie to komórka (5,5))
    int targetRow = mazeSize / 2;
    int targetCol = mazeSize / 2;

    // Tworzymy czujniki: ultradźwiękowy (zasięg 3) oraz dotykowy
    std::shared_ptr<UltrasonicSensor> usensor = std::make_shared<UltrasonicSensor>(3);
    std::shared_ptr<TouchSensor> tsensor = std::make_shared<TouchSensor>();

    // Podpinamy czujniki do robota
    robot.attachSensor(usensor);
    robot.attachSensor(tsensor);

    std::cout << "Start symulacji:" << std::endl;
    printMaze(maze, robot);
    std::cout << "Startowa pozycja robota: (" << robot.getRow() << ", " << robot.getCol() << ")" << std::endl;

    // Maksymalna liczba ruchów, aby uniknąć nieskończonej pętli w przypadku problemów.
    const int maxMoves = 1000;
    int moveCount = 0;

    // Pętla symulacji: wykonuj ruchy aż robot osiągnie cel lub przekroczy maxMoves.
    while ((robot.getRow() != targetRow || robot.getCol() != targetCol) && moveCount < maxMoves) {
        // Wykonaj ruch robota
        robot.move(maze);
        moveCount++;

        // Sprawdź działanie czujników
        bool usDetection = usensor->detect(maze, robot);
        bool tsDetection = tsensor->detect(maze, robot);

        std::cout << "\nPo ruchu " << moveCount << ":" << std::endl;
        std::cout << "Aktualna pozycja robota: (" << robot.getRow() << ", " << robot.getCol() << ")" << std::endl;
        std::cout << "Detekcja czujnika ultradźwiękowego: " << (usDetection ? "Przeszkoda wykryta" : "Brak przeszkody") << std::endl;
        std::cout << "Detekcja czujnika dotykowego: " << (tsDetection ? "Przeszkoda wykryta" : "Brak przeszkody") << std::endl;
        printMaze(maze, robot);
    }

    if (robot.getRow() == targetRow && robot.getCol() == targetCol) {
        std::cout << "\nCel osiągnięty po " << moveCount << " ruchach!" << std::endl;
    } else {
        std::cout << "\nCel nie został osiągnięty po " << moveCount << " ruchach." << std::endl;
    }

    return 0;
}
