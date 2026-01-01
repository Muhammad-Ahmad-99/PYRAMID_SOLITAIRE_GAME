#include "raylib.h"
#include <iostream>
#include <ctime>
using namespace std;

enum GameState {
    MAIN_MENU,
    INSTRUCTIONS,
    PLAYING,
    GAME_OVER
};

class Card {
public:
    int value;
    int suit;
    bool faceUp;
    bool removed;

    Card* next;

    Card(int v = 0, int s = 0) {
        value = v;
        suit = s;
        faceUp = false;
        removed = false;
        next = nullptr;
    }
};

class Deck {
public:
    Card* head;
    Card* tail;

    Deck() {
        head = tail = nullptr;
    }

    void addCard(int v, int s) {
        Card* c = new Card(v, s);

        if (!head)
            head = tail = c;
        else {
            tail->next = c;
            tail = c;
        }
    }

    void shuffle() {
        srand(time(0));
        for (Card* a = head; a; a = a->next) {
            Card* b = head;
            int steps = rand() % 52;

            while (steps-- && b->next)
                b = b->next;

            swap(a->value, b->value);
            swap(a->suit, b->suit);
        }
    }
};

class PyramidSolitaire {
private:
    Deck deck;
    Card* stockTop;

    int score;
    time_t startTime;
    bool gameWon;
    bool gameLost;

    GameState state;

    const int CARD_WIDTH = 90;
    const int CARD_HEIGHT = 130;
    const int CARD_SPACING = 20;

    Texture2D background;
    Texture2D cardTextures[4][13];
    Texture2D stockTexture;

public:
    PyramidSolitaire() {
        state = MAIN_MENU;
        loadCardTextures();
    }

    ~PyramidSolitaire() {
        for (int s = 0; s < 4; s++) {
            for (int v = 0; v < 13; v++) {
                UnloadTexture(cardTextures[s][v]);
            }
        }
        UnloadTexture(background);
        UnloadTexture(stockTexture);
    }

    void loadCardTextures() {
        const char* suits[4] = { "H", "D", "C", "S" };
        const char* values[13] = {
            "A","2","3","4","5","6","7","8","9","10","J","Q","K"
        };

        for (int s = 0; s < 4; s++) {
            for (int v = 0; v < 13; v++) {
                string path = "images/";
                path += values[v];
                path += suits[s];
                path += ".JPG";
                cardTextures[s][v] = LoadTexture(path.c_str());
            }
        }

        stockTexture = LoadTexture("images/stock.jpg");
        background = LoadTexture("images/background.jpg");
    }

    void initGame() {
        deck = Deck();
        createDeck();
        deck.shuffle();
        makePyramid();

        score = 0;
        startTime = time(0);
        gameWon = false;
        gameLost = false;

        state = PLAYING;
    }

    void createDeck() {
        for (int s = 0; s < 4; s++)
            for (int v = 1; v <= 13; v++)
                deck.addCard(v, s);
    }

    void makePyramid() {
        Card* curr = deck.head;

        for (int row = 0; row < 7; row++) {
            for (int col = 0; col <= row; col++) {
                curr->faceUp = true;
                curr = curr->next;
            }
        }
        stockTop = curr;
    }

    void update() {
        if (state == PLAYING) {
            int timeGap = difftime(time(0), startTime);
            if (timeGap > 300) {
                gameLost = true;
                state = GAME_OVER;
            }
        }
    }

    void drawBackground() {
        if (background.id != 0) {
            DrawTexturePro(background,
                { 0, 0, (float)background.width, (float)background.height },
                { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
                { 0, 0 }, 0, WHITE
            );
        }
        else {
            ClearBackground(DARKGREEN);
        }
    }

    Rectangle getPyramidCardRect(int row, int col) {
        int startX = (GetScreenWidth() / 2) - (row * (CARD_WIDTH + CARD_SPACING) / 2);
        int x = startX + col * (CARD_WIDTH + CARD_SPACING);
        int y = 100 + row * (CARD_HEIGHT / 2 + CARD_SPACING);
        return { (float)x, (float)y, (float)CARD_WIDTH, (float)CARD_HEIGHT };
    }

    void drawCard(Card* card, Rectangle rect, bool selected) {
        if (!card)
            return;

        if (!card->faceUp) {
            DrawRectangleRec(rect, DARKBLUE);
            DrawRectangleLinesEx(rect, 2, BLACK);
            DrawText("?", rect.x + rect.width / 2 - 10, rect.y + rect.height / 2 - 10, 30, WHITE);
            return;
        }

        Texture2D tex = cardTextures[card->suit][card->value - 1];

        if (tex.id != 0) {
            DrawTexturePro(
                tex,
                { 0, 0, (float)tex.width, (float)tex.height },
                rect,
                { 0, 0 },
                0,
                WHITE
            );
        }
        else {
            Color suitColor = (card->suit == 0 || card->suit == 1) ? RED : BLACK;
            DrawRectangleRec(rect, WHITE);
            DrawRectangleLinesEx(rect, 2, BLACK);

            const char* values[13] = { "A","2","3","4","5","6","7","8","9","10","J","Q","K" };
            DrawText(values[card->value - 1], rect.x + 10, rect.y + 10, 20, suitColor);

            const char* suitSymbols[4] = { "?", "?", "?", "?" };
            DrawText(suitSymbols[card->suit], rect.x + 10, rect.y + 35, 25, suitColor);
        }

        if (selected) {
            DrawRectangleLinesEx(rect, 3, YELLOW);
        }
    }

    void renderMainMenu() {
        BeginDrawing();
        drawBackground();

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        DrawText("PYRAMID SOLITAIRE", sw / 2 - 250, sh / 2 - 250, 50, GOLD);

        Rectangle playBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 130), 300, 60 };
        Rectangle instructBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 40), 300, 60 };
        Rectangle exitBtn = { (float)(sw / 2 - 150), (float)(sh / 2 + 50), 300, 60 };

        DrawRectangleRec(playBtn, DARKGREEN);
        DrawRectangleLinesEx(playBtn, 3, GREEN);
        DrawText("NEW GAME", sw / 2 - 130, sh / 2 - 110, 25, WHITE);

        DrawRectangleRec(instructBtn, DARKBLUE);
        DrawRectangleLinesEx(instructBtn, 3, BLUE);
        DrawText("INSTRUCTIONS", sw / 2 - 110, sh / 2 - 20, 25, WHITE);

        DrawRectangleRec(exitBtn, DARKGRAY);
        DrawRectangleLinesEx(exitBtn, 3, BLACK);
        DrawText("EXIT GAME", sw / 2 - 80, sh / 2 + 70, 25, WHITE);

        EndDrawing();
    }

    void renderInstructions() {
        BeginDrawing();
        drawBackground();

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        DrawRectangle(sw / 2 - 400, 100, 800, 700, { 0, 0, 0, 200 });
        DrawRectangleLinesEx({ (float)(sw / 2 - 400), 100, 800, 700 }, 3, GOLD);

        DrawText("HOW TO PLAY PYRAMID SOLITAIRE", sw / 2 - 280, 130, 30, GOLD);

        int y = 190;
        int spacing = 35;

        DrawText("OBJECTIVE:", sw / 2 - 350, y, 25, YELLOW);
        y += spacing;
        DrawText("Remove all cards from the pyramid by pairing cards", sw / 2 - 350, y, 20, WHITE);
        y += spacing - 5;
        DrawText("that add up to 13.", sw / 2 - 350, y, 20, WHITE);
        y += spacing + 10;

        DrawText("RULES:", sw / 2 - 350, y, 25, YELLOW);
        y += spacing;
        DrawText("1. Only uncovered cards can be selected.", sw / 2 - 350, y, 20, WHITE);
        y += spacing + 5;
        DrawText("2. Pair two cards that sum to 13:", sw / 2 - 350, y, 20, WHITE);
        y += spacing - 5;
        DrawText("   Ace(1) + Queen(12), 2 + Jack(11), 3 + 10,", sw / 2 - 350, y, 20, WHITE);
        y += spacing - 5;
        DrawText("   4 + 9, 5 + 8, 6 + 7", sw / 2 - 350, y, 20, WHITE);
        y += spacing + 5;
        DrawText("3. Kings (13) can be removed individually.", sw / 2 - 350, y, 20, WHITE);
        y += spacing + 5;
        DrawText("4. Click the STOCK pile to draw cards.", sw / 2 - 350, y, 20, WHITE);
        y += spacing + 10;

        DrawText("SCORING:", sw / 2 - 350, y, 25, YELLOW);
        y += spacing;
        DrawText("King removed: +10 points", sw / 2 - 350, y, 20, WHITE);
        y += spacing - 5;
        DrawText("Pair removed: +20 points", sw / 2 - 350, y, 20, WHITE);

        Rectangle backBtn = { (float)(sw / 2 - 100), (float)(sh - 120), 200, 50 };
        DrawRectangleRec(backBtn, DARKGRAY);
        DrawRectangleLinesEx(backBtn, 2, WHITE);
        DrawText("BACK TO MENU", sw / 2 - 85, sh - 105, 20, WHITE);

        EndDrawing();
    }

    void drawHUD() {
        int timeGap = difftime(time(0), startTime);
        int sw = GetScreenWidth();

        DrawText("PYRAMID SOLITAIRE GAME", sw / 2 - 200, 20, 30, YELLOW);
        DrawText(TextFormat("Score: %d", score), 20, 60, 20, RAYWHITE);

        int hours = timeGap / 3600;
        int minutes = (timeGap % 3600) / 60;
        int seconds = timeGap % 60;
        DrawText(TextFormat("Time: %02d:%02d:%02d", hours, minutes, seconds), 20, 90, 20, RAYWHITE);
    }

    void drawPyramidCards() {
        Card* curr = deck.head;
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col <= row; col++) {
                if (!curr)
                    break;

                if (!curr->removed) {
                    Rectangle rect = getPyramidCardRect(row, col);
                    drawCard(curr, rect, false);
                }

                curr = curr->next;
            }
        }
    }

    void drawStockPile() {
        int uiStartY = 100 + 7 * (CARD_HEIGHT / 2 + CARD_SPACING);
        Rectangle stockRect = { 180.0f, (float)uiStartY, (float)CARD_WIDTH, (float)CARD_HEIGHT };

        DrawText("STOCK", 180, uiStartY - 30, 20, WHITE);

        if (stockTexture.id != 0) {
            DrawTexturePro(
                stockTexture,
                { 0, 0, (float)stockTexture.width, (float)stockTexture.height },
                stockRect,
                { 0, 0 }, 0, WHITE
            );
        }
        else {
            DrawRectangleRec(stockRect, BLUE);
            DrawRectangleLinesEx(stockRect, 2, WHITE);
        }
    }

    void drawRestartButton() {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        Rectangle restartBtn = { (float)(sw - 150), (float)(sh - 60), 120, 50 };
        DrawRectangleRec(restartBtn, MAROON);
        DrawRectangleLinesEx(restartBtn, 2, WHITE);
        DrawText("RESTART", sw - 140, sh - 45, 20, WHITE);
    }

    void renderGameOver() {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });

        if (gameWon) {
            DrawText("YOU WIN!", sw / 2 - 100, sh / 2 - 50, 40, GOLD);
        }
        else {
            DrawText("GAME OVER", sw / 2 - 120, sh / 2 - 50, 40, RED);
        }

        DrawText(TextFormat("Final Score: %d", score), sw / 2 - 100, sh / 2 + 10, 30, WHITE);
        DrawText("Press R to Restart | M for Menu", sw / 2 - 180, sh / 2 + 60, 20, WHITE);
    }

    void render() {
        if (state == MAIN_MENU) {
            renderMainMenu();
            return;
        }

        if (state == INSTRUCTIONS) {
            renderInstructions();
            return;
        }

        BeginDrawing();
        drawBackground();

        if (state == PLAYING) {
            drawHUD();
            drawPyramidCards();
            drawStockPile();
            drawRestartButton();

            if (gameWon || gameLost) {
                renderGameOver();
            }
        }

        EndDrawing();
    }

    void handleInput() {
        if (state == MAIN_MENU) {
            if (IsKeyPressed(KEY_ENTER))
                initGame();
            if (IsKeyPressed(KEY_I))
                state = INSTRUCTIONS;
        }
        else if (state == INSTRUCTIONS) {
            if (IsKeyPressed(KEY_BACKSPACE))
                state = MAIN_MENU;
        }
        else if (state == GAME_OVER || gameWon || gameLost) {
            if (IsKeyPressed(KEY_R))
                initGame();
            if (IsKeyPressed(KEY_M))
                state = MAIN_MENU;
        }
    }
};

int main() {
    InitWindow(1200, 800, "Pyramid Solitaire");
    SetTargetFPS(60);

    PyramidSolitaire game;

    while (!WindowShouldClose()) {
        game.handleInput();
        game.update();

        BeginDrawing();
        game.render();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
