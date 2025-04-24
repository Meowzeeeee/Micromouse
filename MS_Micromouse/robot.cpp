#include "robot.h"
#include "touch_sensor.h"
#include "rotating_ultrasonic_sensor.h"
#include <climits>
#include <queue>
#include <algorithm>
#include <QDebug>

Robot::~Robot() = default;

Robot::Robot(int startRow,
             int startCol,
             Direction startDir,
             MovementAlgorithm algo,
             int goalRow,
             int goalCol)
    : m_row(startRow)
    , m_col(startCol)
    , m_direction(startDir)
    , m_touchSensor(std::make_unique<TouchSensor>())
    , m_rotSonar(std::make_unique<RotatingUltrasonicSensor>(2))
    , m_algorithm(algo)
    , m_goalRow(goalRow)
    , m_goalCol(goalCol)
{}

void Robot::move(const Maze &maze) {
    switch (m_algorithm) {
    case MovementAlgorithm::Sensors:  moveWithSensors(maze); break;
    case MovementAlgorithm::FloodFill: moveFloodFill(maze);  break;
    case MovementAlgorithm::AStar:     moveAStar(maze);      break;
    }
}

// Zzujnikowy wall-follower
void Robot::moveWithSensors(const Maze &maze) {
    static int stepCounter = 0;
    stepCounter++;

    // Debug: pozycja i ściany
    qDebug() << "Robot at (" << m_row << "," << m_col << ") facing" << int(m_direction);
    qDebug() << "Maze walls at (" << m_row << "," << m_col << "):"
             << "Up="    << maze.hasWall(m_row, m_col, Direction::Up)
             << " Right=" << maze.hasWall(m_row, m_col, Direction::Right)
             << " Down="  << maze.hasWall(m_row, m_col, Direction::Down)
             << " Left="  << maze.hasWall(m_row, m_col, Direction::Left);

    // 1) TouchSensor (front)
    bool frontBlocked = m_touchSensor->detect(maze, *this);
    if (frontBlocked) {
        qDebug() << "Touch sensor blocks front.";
    }

    // 2) RotatingUltrasonicSensor (co 3 kroki)
    bool useSonar = (stepCounter % 3 == 0);
    bool leftBlocked = false, rightBlocked = false;
    if (useSonar) {
        m_rotSonar->update(maze, *this);
        int distLeft  = m_rotSonar->getDistance(Direction::Left);
        int distRight = m_rotSonar->getDistance(Direction::Right);
        leftBlocked  = (distLeft < 1);
        rightBlocked = (distRight < 1);
        qDebug() << "Sonar side check: L=" << distLeft << " R=" << distRight
                 << " leftBlocked=" << leftBlocked << " rightBlocked=" << rightBlocked;
    }

    // 3) Obliczenie kierunków i koordynatów
    Direction leftDir, rightDir, frontDir;
    int forwardRow, forwardCol, leftRow, leftCol, rightRow, rightCol;
    switch (m_direction) {
    case Direction::Up:
        frontDir    = Direction::Up;    forwardRow = m_row - 1; forwardCol = m_col;
        leftDir     = Direction::Left;  leftRow    = m_row;     leftCol   = m_col - 1;
        rightDir    = Direction::Right; rightRow   = m_row;     rightCol  = m_col + 1;
        break;
    case Direction::Right:
        frontDir    = Direction::Right; forwardRow = m_row;     forwardCol = m_col + 1;
        leftDir     = Direction::Up;    leftRow    = m_row - 1; leftCol   = m_col;
        rightDir    = Direction::Down;  rightRow   = m_row + 1; rightCol  = m_col;
        break;
    case Direction::Down:
        frontDir    = Direction::Down;  forwardRow = m_row + 1; forwardCol = m_col;
        leftDir     = Direction::Right; leftRow    = m_row;     leftCol   = m_col + 1;
        rightDir    = Direction::Left;  rightRow   = m_row;     rightCol  = m_col - 1;
        break;
    case Direction::Left:
        frontDir    = Direction::Left;  forwardRow = m_row;     forwardCol = m_col - 1;
        leftDir     = Direction::Down;  leftRow    = m_row + 1; leftCol   = m_col;
        rightDir    = Direction::Up;    rightRow   = m_row - 1; rightCol  = m_col;
        break;
    }

    // 4) Wall-follower (lewa ręka)
    if (!maze.hasWall(m_row, m_col, leftDir) && !leftBlocked) {
        qDebug() << "Decision: Turn left";
        turnLeft();
        m_row = leftRow;
        m_col = leftCol;
        return;
    }
    if (!frontBlocked && !maze.hasWall(m_row, m_col, frontDir)) {
        qDebug() << "Decision: Move forward";
        m_row = forwardRow;
        m_col = forwardCol;
        return;
    }
    if (!maze.hasWall(m_row, m_col, rightDir) && !rightBlocked) {
        qDebug() << "Decision: Turn right";
        turnRight();
        m_row = rightRow;
        m_col = rightCol;
        return;
    }
    qDebug() << "Decision: Turn around (dead end)";
    turnRight();
    turnRight();
}

// Flood-Fill: wyznaczanie gradientu od celu i podążanie nim
void Robot::moveFloodFill(const Maze &maze) {
    // 1) Pobierz wymiary labiryntu
    int W = maze.getWidth();
    int H = maze.getHeight();

    // 2) Zainicjuj tablicę odległości dużymi wartościami
    std::vector<std::vector<int>> dist(H, std::vector<int>(W, INT_MAX));

    // 3) BFS z poziomu celu (m_goalRow, m_goalCol), dystans do siebie = 0
    std::queue<std::pair<int,int>> q;
    dist[m_goalRow][m_goalCol] = 0;
    q.push({m_goalRow, m_goalCol});

    // Kierunki poruszania się (Up, Right, Down, Left)
    std::array<Direction,4> dirs = {
        Direction::Up, Direction::Right,
        Direction::Down, Direction::Left
    };
    int dr[4] = {-1, 0, 1, 0};
    int dc[4] = { 0, 1, 0,-1};

    // 4) Wypełnianie dist przez BFS
    while (!q.empty()) {
        auto [r, c] = q.front(); q.pop();
        for (int i = 0; i < 4; ++i) {
            int nr = r + dr[i];
            int nc = c + dc[i];
            // poza granicami?
            if (nr < 0 || nr >= H || nc < 0 || nc >= W) continue;
            // ściana blokuje?
            if (maze.hasWall(r, c, dirs[i])) continue;
            // relaksacja dystansu
            if (dist[nr][nc] > dist[r][c] + 1) {
                dist[nr][nc] = dist[r][c] + 1;
                q.push({nr, nc});
            }
        }
    }

    // 5) Wybór ruchu: spośród sąsiadów idź tam, gdzie dist jest najmniejsze
    int best = dist[m_row][m_col];
    int choice = -1;
    int nr = m_row, nc = m_col;
    for (int i = 0; i < 4; ++i) {
        int rr = m_row + dr[i];
        int cc = m_col + dc[i];
        if (rr < 0 || rr >= H || cc < 0 || cc >= W) continue;
        if (maze.hasWall(m_row, m_col, dirs[i])) continue;
        if (dist[rr][cc] < best) {
            best   = dist[rr][cc];
            choice = i;
            nr = rr;
            nc = cc;
        }
    }

    // 6) Jeśli znaleziono kierunek z mniejszym dist, wykonaj ruch
    if (choice >= 0) {
        m_direction = dirs[choice];
        m_row       = nr;
        m_col       = nc;
    }
    // Jeśli choice == -1, robot stoi w martwym punkcie lub już jest w celu
}


// A*: jednokrotne planowanie trasy + podążanie po niej
void Robot::moveAStar(const Maze &maze) {
    int W = maze.getWidth();
    int H = maze.getHeight();

    // 1) Jeśli nie mamy jeszcze wyliczonej ścieżki lub doszliśmy do jej końca → zaplanuj na nowo
    if (m_path.empty() || m_pathIndex >= m_path.size()) {
        // Struktura węzła dla kolejki A*
        struct Node { int r, c, g, f; };
        // Kolejka priorytetowa (min-heap) po wartości f = g + h
        auto cmp = [](auto &a, auto &b){ return a.f > b.f; };
        std::priority_queue<Node, std::vector<Node>, decltype(cmp)> open(cmp);

        // Tablice pomocnicze:
        // closed[r][c] — czy już odwiedziliśmy
        std::vector<std::vector<bool>> closed(H, std::vector<bool>(W, false));
        // gScore[r][c] — najlepszy koszt dojścia z startu do (r,c)
        std::vector<std::vector<int>> gScore(H, std::vector<int>(W, INT_MAX));
        // from[r][c] — stąd przyszliśmy do (r,c) (rekonstrukcja ścieżki)
        std::vector<std::vector<std::pair<int,int>>> from(
            H, std::vector<std::pair<int,int>>(W, {-1,-1})
            );

        // Heurystyka: Manhattan distance do celu
        auto h = [&](int r, int c){
            return std::abs(r - m_goalRow) + std::abs(c - m_goalCol);
        };

        // 1.1) Zainicjuj startowy węzeł
        gScore[m_row][m_col] = 0;
        open.push({ m_row, m_col, 0, h(m_row, m_col) });

        // Kierunki: Up, Right, Down, Left
        std::array<Direction,4> dirs = {
            Direction::Up, Direction::Right,
            Direction::Down, Direction::Left
        };
        int dr[4] = {-1, 0, 1, 0};
        int dc[4] = { 0, 1, 0,-1};

        // 1.2) Główna pętla A*
        while (!open.empty()) {
            Node cur = open.top();
            open.pop();

            // Pomijaj, jeśli już zamknięty
            if (closed[cur.r][cur.c]) continue;
            closed[cur.r][cur.c] = true;

            // Jeśli dotarliśmy do celu → przerwij
            if (cur.r == m_goalRow && cur.c == m_goalCol) break;

            // Rozwiń sąsiadów
            for (int i = 0; i < 4; ++i) {
                int rr = cur.r + dr[i];
                int cc = cur.c + dc[i];
                // poza granicami?
                if (rr < 0 || rr >= H || cc < 0 || cc >= W) continue;
                // ściana między (cur.r,cur.c) a (rr,cc)?
                if (maze.hasWall(cur.r, cur.c, dirs[i])) continue;

                int ng = cur.g + 1;  // koszt do sąsiada
                if (ng < gScore[rr][cc]) {
                    // lepszy koszt → zaktualizuj
                    gScore[rr][cc] = ng;
                    int fscore = ng + h(rr, cc);
                    open.push({ rr, cc, ng, fscore });
                    from[rr][cc] = { cur.r, cur.c };
                }
            }
        }

        // 2) Rekonstrukcja ścieżki od celu do startu
        m_path.clear();
        std::pair<int,int> p = { m_goalRow, m_goalCol };
        while (!(p.first == m_row && p.second == m_col)) {
            m_path.push_back(p);
            p = from[p.first][p.second];
        }
        std::reverse(m_path.begin(), m_path.end());
        m_pathIndex = 0;
    }

    // 3) Wykonaj kolejny krok po wyliczonej ścieżce
    if (m_pathIndex < m_path.size()) {
        auto [tr, tc] = m_path[m_pathIndex++];
        // Ustaw nowy kierunek na podstawie różnicy współrzędnych
        if      (tr == m_row - 1) m_direction = Direction::Up;
        else if (tr == m_row + 1) m_direction = Direction::Down;
        else if (tc == m_col + 1) m_direction = Direction::Right;
        else if (tc == m_col - 1) m_direction = Direction::Left;

        // Przesuń robota
        m_row = tr;
        m_col = tc;
    }
}

void Robot::turnLeft() {
    switch (m_direction) {
    case Direction::Up:    m_direction=Direction::Left;  break;
    case Direction::Right: m_direction=Direction::Up;    break;
    case Direction::Down:  m_direction=Direction::Right; break;
    case Direction::Left:  m_direction=Direction::Down;  break;
    }
}

void Robot::turnRight() {
    switch (m_direction) {
    case Direction::Up:    m_direction=Direction::Right; break;
    case Direction::Right: m_direction=Direction::Down;  break;
    case Direction::Down:  m_direction=Direction::Left;  break;
    case Direction::Left:  m_direction=Direction::Up;    break;
    }
}
