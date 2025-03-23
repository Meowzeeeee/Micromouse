#ifndef MAZE_H
#define MAZE_H

#include <vector>

struct Cell {
    int row;
    int col;
    bool topWall;
    bool rightWall;
    bool bottomWall;
    bool leftWall;
    bool visited;
};

class Maze
{
public:
    Maze(int width, int height);
    void generate();  // generowanie labiryntu
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    bool hasTopWall(int row, int col) const;
    bool hasRightWall(int row, int col) const;
    bool hasBottomWall(int row, int col) const;
    bool hasLeftWall(int row, int col) const;

    // Opcjonalnie można dodać metodę, która zwróci konkretną komórkę:
    const Cell& getCell(int row, int col) const;

private:
    int m_width;
    int m_height;
    std::vector<std::vector<Cell>> m_cells;
    void initCells();
    void carveMaze(int row, int col);
};

#endif // MAZE_H
