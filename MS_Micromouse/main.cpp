#include <iostream>
#include "maze.h"
#include "robot.h"

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
    int mazeSize = 10; // Rozmiar labiryntu, który można później ustalać przez interfejs
    Maze maze(mazeSize, mazeSize);
    Robot robot(0, 0);  // Początkowa pozycja robota, np. lewy górny róg (0,0)

    std::cout << "Początkowy labirynt z robotem:" << std::endl;
    printMaze(maze, robot);
    std::cout << "Pozycja robota: (" << robot.getRow() << ", " << robot.getCol() << ")" << std::endl;

    // Symulujemy 20 ruchów robota
    for (int i = 0; i < 20; ++i) {
        robot.move(maze);
        std::cout << "\nPo ruchu " << i + 1 << ":" << std::endl;
        printMaze(maze, robot);
        std::cout << "Pozycja robota: (" << robot.getRow() << ", " << robot.getCol() << ")" << std::endl;
    }

    return 0;
}
