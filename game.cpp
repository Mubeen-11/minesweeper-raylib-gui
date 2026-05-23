#include "raylib.h"
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <algorithm>

using namespace std;

// ── Constants ────────────────────────────────────────────────────────────────
const int BOARD_SIZE = 9;
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 820;
const int CELL_SIZE = 56;
const int CELL_GAP = 4;

const int MIN_MINES = 1;
const int MAX_MINES = 20;

// ── State ────────────────────────────────────────────────────────────────────
int  minesCount = 10;
int  board[BOARD_SIZE][BOARD_SIZE];
bool revealed[BOARD_SIZE][BOARD_SIZE];
bool flagged[BOARD_SIZE][BOARD_SIZE];

bool gameOver = false;
bool win = false;
bool typing = true;
bool exitGame = false;

string username = "";
string message = "Enter your username and press ENTER";

// ── Palette ──────────────────────────────────────────────────────────────────
Color C_BG = { 9,  18,  36, 255 };
Color C_PANEL = { 16,  34,  60, 255 };
Color C_BOARD = { 10,  22,  44, 255 };
Color C_CELL = { 58,  68, 102, 255 };
Color C_HOVER = { 78,  90, 132, 255 };
Color C_REVEALED = { 175, 188, 205, 255 };
Color C_ACCENT = { 88, 166, 255, 255 };

// ── Board logic ──────────────────────────────────────────────────────────────
bool fileExists(const string& f) { ifstream in(f); return in.good(); }

void startup()
{
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            board[i][j] = revealed[i][j] = flagged[i][j] = 0;
}

void placeMines()
{
    int placed = 0;
    while (placed < minesCount)
    {
        int x = rand() % BOARD_SIZE, y = rand() % BOARD_SIZE;
        if (board[x][y] != -1) { board[x][y] = -1; placed++; }
    }
}

void calcNumbers()
{
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j] != -1)
                for (int x = i - 1; x <= i + 1; x++)
                    for (int y = j - 1; y <= j + 1; y++)
                        if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && board[x][y] == -1)
                            board[i][j]++;
}

void newGame()
{
    startup(); placeMines(); calcNumbers();
    gameOver = win = false;
    message = "New game — good luck!";
}

void revealCell(int x, int y)
{
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) return;
    if (revealed[x][y] || flagged[x][y] || gameOver || win) return;
    revealed[x][y] = true;
    if (board[x][y] == -1)
    {
        gameOver = true;
        message = "Boom! Mine detonated.";
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++)
                if (board[i][j] == -1) revealed[i][j] = true;
        return;
    }
    if (board[x][y] == 0)
        for (int i = x - 1; i <= x + 1; i++)
            for (int j = y - 1; j <= y + 1; j++)
                revealCell(i, j);
}

void checkWin()
{
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j] != -1 && !revealed[i][j]) return;
    win = true; message = "Board cleared — you win!";
}

int countFlags()
{
    int n = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (flagged[i][j]) n++;
    return n;
}

// ── Save / Load ──────────────────────────────────────────────────────────────
void saveGame(const string& fn)
{
    ofstream out(fn);
    if (!out) { message = "Save failed"; return; }
    out << minesCount << "\n";
    for (int i = 0; i < BOARD_SIZE; i++) { for (int j = 0; j < BOARD_SIZE; j++) out << board[i][j] << " "; out << "\n"; }
    for (int i = 0; i < BOARD_SIZE; i++) { for (int j = 0; j < BOARD_SIZE; j++) out << revealed[i][j] << " "; out << "\n"; }
    for (int i = 0; i < BOARD_SIZE; i++) { for (int j = 0; j < BOARD_SIZE; j++) out << flagged[i][j] << " "; out << "\n"; }
    out.close(); message = "Game saved!";
}

void loadGame(const string& fn)
{
    ifstream in(fn);
    if (!in) { message = "No saved game found"; return; }
    in >> minesCount;
    for (int i = 0; i < BOARD_SIZE; i++) for (int j = 0; j < BOARD_SIZE; j++) in >> board[i][j];
    for (int i = 0; i < BOARD_SIZE; i++) for (int j = 0; j < BOARD_SIZE; j++) in >> revealed[i][j];
    for (int i = 0; i < BOARD_SIZE; i++) for (int j = 0; j < BOARD_SIZE; j++) in >> flagged[i][j];
    in.close(); gameOver = win = false; message = "Game loaded!";
}

// ── Drawing ──────────────────────────────────────────────────────────────────
Color numColor(int n)
{
    Color t[] = { {0,0,0,0},
        {80,160,255,255},{60,210,100,255},{255,80,80,255},
        {140,100,255,255},{255,120,60,255},{0,210,220,255},
        {220,80,220,255},{200,200,200,255} };
    return (n >= 1 && n <= 8) ? t[n] : BLACK;
}

// Centered text helper
void drawTextC(const char* text, int cx, int y, int fs, Color col)
{
    DrawText(text, cx - MeasureText(text, fs) / 2, y, fs, col);
}

void drawPanel(Rectangle r, Color fill, Color border)
{
    DrawRectangleRounded(r, 0.12f, 16, fill);
    DrawRectangleRoundedLines(r, 0.12f, 16, border);
}

void drawButton(Rectangle r, const char* label, Color col)
{
    bool hov = CheckCollisionPointRec(GetMousePosition(), r);
    Color bg = hov ? Fade(col, 0.78f) : col;
    DrawRectangleRounded({ r.x + 3,r.y + 4,r.width,r.height }, 0.18f, 12, Fade(BLACK, 0.30f));
    DrawRectangleRounded(r, 0.18f, 12, bg);
    DrawRectangleRoundedLines(r, 0.18f, 12, Fade(WHITE, 0.55f));
    int fs = 22; drawTextC(label, (int)(r.x + r.width / 2), (int)(r.y + r.height / 2 - fs / 2), fs, WHITE);
}

bool buttonPressed(Rectangle r)
{
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), r);
}

void drawSmallBtn(Rectangle r, const char* label, Color col)
{
    bool hov = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRounded(r, 0.25f, 12, hov ? Fade(col, 0.85f) : col);
    DrawRectangleRoundedLines(r, 0.25f, 12, Fade(WHITE, 0.50f));
    int fs = 20; drawTextC(label, (int)(r.x + r.width / 2), (int)(r.y + r.height / 2 - fs / 2), fs, WHITE);
}

void drawMine(int cx, int cy, bool hit)
{
    Color body = hit ? Color{ 230,50,60,255 } : Color{ 30,30,30,255 };
    Color spk = hit ? Color{ 160,20,20,255 } : Color{ 60,60,60,255 };
    DrawCircle(cx, cy, 13, body);
    DrawCircle(cx - 4, cy - 4, 3, Fade(WHITE, 0.35f));
    for (int a = 0; a < 4; a++) {
        int dx[] = { 18,0,-18,0 }, dy[] = { 0,18,0,-18 };
        DrawLine(cx, cy, cx + dx[a], cy + dy[a], spk);
        DrawCircle(cx + dx[a], cy + dy[a], 3, spk);
    }
    for (int a = 0; a < 4; a++) {
        int dx[] = { 12,12,-12,-12 }, dy[] = { -12,12,12,-12 };
        DrawLine(cx, cy, cx + dx[a], cy + dy[a], spk);
    }
}

void drawFlag(int cx, int cy)
{
    DrawLine(cx, cy + 15, cx, cy - 12, WHITE);
    DrawTriangle({ (float)cx,(float)(cy - 12) },
        { (float)cx,(float)(cy + 1) },
        { (float)(cx + 13),(float)(cy - 6) }, RED);
    DrawLineEx({ (float)(cx - 8),(float)(cy + 15) }, { (float)(cx + 8),(float)(cy + 15) }, 2, WHITE);
}

void drawOverlay()
{
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.70f));
    Rectangle box = { SCREEN_WIDTH / 2 - 220, SCREEN_HEIGHT / 2 - 130, 440, 260 };
    DrawRectangleRounded(box, 0.10f, 20, { 12,20,36,250 });
    Color border = gameOver ? RED : GREEN;
    DrawRectangleRoundedLines(box, 0.10f, 20, border);

    if (gameOver)
    {
        drawTextC("GAME OVER", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 80, 52, RED);
        drawTextC("You hit a mine.", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 10, 24, LIGHTGRAY);
    }
    else
    {
        drawTextC("YOU WIN!", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 80, 56, GREEN);
        drawTextC("All safe cells revealed!", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 10, 24, LIGHTGRAY);
    }
    drawTextC("Press NEW GAME to play again", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 55, 20, { 150,160,180,255 });
}

// ── Main ─────────────────────────────────────────────────────────────────────
int main()
{
    srand((unsigned int)time(0));
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Minesweeper");
    SetTargetFPS(60);
    SetWindowFocused();
    SetWindowPosition(
        GetMonitorWidth(GetCurrentMonitor()) / 2 - SCREEN_WIDTH / 2,
        GetMonitorHeight(GetCurrentMonitor()) / 2 - SCREEN_HEIGHT / 2
    );

    startup();

    // ── Layout math (all computed, never hardcoded) ──────────────────────────
    int boardPixels = BOARD_SIZE * CELL_SIZE + (BOARD_SIZE - 1) * CELL_GAP;

    // Board centered in the left ~75% of screen
    int playAreaW = 920;
    int boardX = (playAreaW - boardPixels) / 2 + 10;
    int boardY = 190;

    // Right sidebar
    int sideX = 950;
    int sideW = 220;
    int btnH = 58;
    int btnGap = 14;

    Rectangle saveBtn = { (float)sideX, 200,          (float)sideW, (float)btnH };
    Rectangle loadBtn = { (float)sideX, 200 + btnH + btnGap,    (float)sideW, (float)btnH };
    Rectangle newBtn = { (float)sideX, 200 + (btnH + btnGap) * 2,(float)sideW, (float)btnH };
    Rectangle exitBtn = { (float)sideX, 200 + (btnH + btnGap) * 3,(float)sideW, (float)btnH };

    // Mine adjuster buttons
    Rectangle minusBtn = { (float)sideX,        560, 62, 46 };
    Rectangle plusBtn = { (float)(sideX + sideW - 62), 560, 62, 46 };

    while (!WindowShouldClose() && !exitGame)
    {
        Vector2 mouse = GetMousePosition();

        // ── Username input ───────────────────────────────────────────────────
        if (typing)
        {
            int key = GetCharPressed();
            while (key > 0)
            {
                if (key >= 32 && key <= 125 && (int)username.size() < 18) username += (char)key;
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !username.empty()) username.pop_back();
            if (IsKeyPressed(KEY_ENTER))
            {
                if (!username.empty())
                {
                    typing = false;
                    if (fileExists(username + ".txt")) loadGame(username + ".txt");
                    else { newGame(); message = "Welcome, " + username + "!"; }
                }
                else message = "Please enter a username first.";
            }
        }

        // ── Left click ───────────────────────────────────────────────────────
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !typing)
        {
            // Board cells
            if (!gameOver && !win)
                for (int i = 0; i < BOARD_SIZE; i++)
                    for (int j = 0; j < BOARD_SIZE; j++)
                    {
                        Rectangle cell = { (float)(boardX + j * (CELL_SIZE + CELL_GAP)),
                                        (float)(boardY + i * (CELL_SIZE + CELL_GAP)),
                                        (float)CELL_SIZE,(float)CELL_SIZE };
                        if (CheckCollisionPointRec(mouse, cell))
                        {
                            revealCell(i, j); checkWin();
                        }
                    }

            // Buttons
            if (buttonPressed(saveBtn)) saveGame(username + ".txt");
            if (buttonPressed(loadBtn)) loadGame(username + ".txt");
            if (buttonPressed(newBtn)) { newGame(); if (typing) typing = false; }
            if (buttonPressed(exitBtn)) exitGame = true;

            // Mine count adjuster
            if (buttonPressed(minusBtn))
            {
                minesCount = max(MIN_MINES, minesCount - 1);
                message = TextFormat("Mines set to %d — press NEW GAME", minesCount);
            }
            if (buttonPressed(plusBtn))
            {
                minesCount = min(MAX_MINES, minesCount + 1);
                message = TextFormat("Mines set to %d — press NEW GAME", minesCount);
            }
        }

        // ── Right click (flag) ───────────────────────────────────────────────
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !typing && !gameOver && !win)
            for (int i = 0; i < BOARD_SIZE; i++)
                for (int j = 0; j < BOARD_SIZE; j++)
                {
                    Rectangle cell = { (float)(boardX + j * (CELL_SIZE + CELL_GAP)),
                                    (float)(boardY + i * (CELL_SIZE + CELL_GAP)),
                                    (float)CELL_SIZE,(float)CELL_SIZE };
                    if (CheckCollisionPointRec(mouse, cell) && !revealed[i][j])
                    {
                        flagged[i][j] = !flagged[i][j];
                        message = TextFormat("Flags: %d / %d mines", countFlags(), minesCount);
                    }
                }

        // ────────────────────────────────────────────────────────────────────
        BeginDrawing();
        ClearBackground(C_BG);

        // ── Title ────────────────────────────────────────────────────────────
        {
            const char* t = "MINESWEEPER";
            int fs = 54;
            int tx = SCREEN_WIDTH / 2 - MeasureText(t, fs) / 2;
            // subtle shadow
            DrawText(t, tx + 3, 23, fs, Fade(BLACK, 0.50f));
            DrawText(t, tx, 20, fs, GOLD);
            // underline
            DrawRectangle(SCREEN_WIDTH / 2 - 180, 78, 360, 3, Fade(GOLD, 0.35f));
        }

        // ── Main panel ───────────────────────────────────────────────────────
        drawPanel({ 90,100,1020,690 }, C_PANEL, Fade(WHITE, 0.12f));

        // ── Status bar ───────────────────────────────────────────────────────
        {
            Rectangle sb = { 110,108,980,38 };
            DrawRectangleRounded(sb, 0.35f, 12, { 0,0,0,80 });
            drawTextC(message.c_str(), SCREEN_WIDTH / 2, 118, 20, { 180,195,215,255 });
        }

        // ── Header widgets: Mines | Username ─────────────────────────────────
        // Mines panel
        {
            drawPanel({ 110,155,200,80 }, C_BOARD, Fade(C_ACCENT, 0.25f));
            DrawText("MINES", 163, 163, 16, { 140,160,190,255 });
            const char* mv = TextFormat("%d", minesCount - countFlags());
            int mfs = 36;
            DrawText(mv, 210 - MeasureText(mv, mfs) / 2, 181, mfs, { 255,100,100,255 });
        }

        // Player panel
        {
            drawPanel({ 870,155,240,80 }, C_BOARD, Fade(C_ACCENT, 0.25f));
            DrawText("PLAYER", 930, 163, 16, { 140,160,190,255 });
            const char* pn = username.empty() ? "---" : username.c_str();
            DrawText(pn, 990 - MeasureText(pn, 22) / 2, 184, 22, C_ACCENT);
        }

        // Username input box (centered in the gap between panels)
        if (typing)
        {
            Rectangle ib = { 360,155,360,80 };
            DrawRectangleRounded(ib, 0.12f, 12, { 230,235,245,255 });
            DrawRectangleRoundedLines(ib, 0.12f, 12, C_ACCENT);
            DrawText("Username:", 376, 165, 16, { 100,110,130,255 });
            string shown = username + ((int)(GetTime() * 2) % 2 == 0 ? "|" : "");
            DrawText(shown.c_str(), 376, 190, 26, { 20,30,50,255 });
        }

        // ── Board background ─────────────────────────────────────────────────
        {
            int pad = 20;
            drawPanel(
                { (float)(boardX - pad),(float)(boardY - pad),
                 (float)(boardPixels + pad * 2),(float)(boardPixels + pad * 2) },
                C_BOARD, Fade(WHITE, 0.10f));
        }

        // Column labels
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            int lx = boardX + j * (CELL_SIZE + CELL_GAP) + CELL_SIZE / 2;
            drawTextC(TextFormat("%d", j), lx, boardY - 28, 18, { 120,140,170,255 });
        }
        // Row labels
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            int ly = boardY + i * (CELL_SIZE + CELL_GAP) + CELL_SIZE / 2 - 9;
            DrawText(TextFormat("%d", i), boardX - 32, ly, 18, { 120,140,170,255 });
        }

        // ── Cells ────────────────────────────────────────────────────────────
        for (int i = 0; i < BOARD_SIZE; i++)
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                int cx = boardX + j * (CELL_SIZE + CELL_GAP);
                int cy = boardY + i * (CELL_SIZE + CELL_GAP);
                Rectangle cell = { (float)cx,(float)cy,(float)CELL_SIZE,(float)CELL_SIZE };
                bool hov = CheckCollisionPointRec(mouse, cell);

                if (revealed[i][j])
                {
                    bool mine = (board[i][j] == -1);
                    DrawRectangleRounded(cell, 0.10f, 8,
                        mine ? Color{ 180,30,40,255 } : C_REVEALED);
                    DrawRectangleRoundedLines(cell, 0.10f, 8, Fade(BLACK, 0.25f));
                    if (mine)
                        drawMine(cx + CELL_SIZE / 2, cy + CELL_SIZE / 2, true);
                    else if (board[i][j] > 0)
                    {
                        const char* n = TextFormat("%d", board[i][j]);
                        int nfs = 32;
                        DrawText(n, cx + CELL_SIZE / 2 - MeasureText(n, nfs) / 2,
                            cy + CELL_SIZE / 2 - nfs / 2 + 1, nfs, numColor(board[i][j]));
                    }
                }
                else if (flagged[i][j])
                {
                    DrawRectangleRounded(cell, 0.10f, 8, { 55,62,95,255 });
                    DrawRectangleRoundedLines(cell, 0.10f, 8, Fade(RED, 0.85f));
                    drawFlag(cx + CELL_SIZE / 2, cy + CELL_SIZE / 2);
                }
                else
                {
                    Color bg = hov ? C_HOVER : C_CELL;
                    DrawRectangleRounded(cell, 0.10f, 8, bg);
                    DrawRectangleRoundedLines(cell, 0.10f, 8, Fade(WHITE, 0.18f));
                    // top-edge sheen
                    DrawRectangleRounded({ (float)(cx + 5),(float)(cy + 4),(float)(CELL_SIZE - 10),5 },
                        0.5f, 4, Fade(WHITE, 0.12f));
                }
            }

        // ── Sidebar buttons ──────────────────────────────────────────────────
        drawButton(saveBtn, "SAVE GAME", { 30,160, 60,255 });
        drawButton(loadBtn, "LOAD GAME", { 30,110,210,255 });
        drawButton(newBtn, "NEW GAME", { 220,150, 20,255 });
        drawButton(exitBtn, "EXIT GAME", { 210, 45, 55,255 });

        // ── Mine adjuster ────────────────────────────────────────────────────
        {
            // Label above
            drawTextC("MINE COUNT", sideX + sideW / 2, 538, 16, { 140,155,180,255 });

            // Big number in middle
            drawPanel({ (float)sideX, 556, (float)sideW, 50 }, C_BOARD, Fade(WHITE, 0.10f));
            drawTextC(TextFormat("%d", minesCount), sideX + sideW / 2, 563, 30, { 255,200,80,255 });

            // – and + buttons over the panel (left/right ends)
            drawSmallBtn(minusBtn, "-", { 150,50,50,255 });
            drawSmallBtn(plusBtn, "+", { 50,130,80,255 });
        }

        // ── Hint strip ───────────────────────────────────────────────────────
        {
            drawPanel({ (float)sideX,620,(float)sideW,72 }, C_BOARD, Fade(WHITE, 0.08f));
            DrawText("L-click  reveal cell", sideX + 12, 632, 16, { 130,150,175,255 });
            DrawText("R-click  place flag", sideX + 12, 655, 16, { 130,150,175,255 });
            DrawText("Flag all mines to win", sideX + 12, 678, 14, { 90,110,140,255 });
        }

        // ── Game over / win overlay ──────────────────────────────────────────
        if (gameOver || win) drawOverlay();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
