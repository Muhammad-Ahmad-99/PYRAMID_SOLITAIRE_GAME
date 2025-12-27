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

public:
    PyramidSolitaire() {
        state = MAIN_MENU;
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

    void render() {
        ClearBackground(DARKGREEN);

        switch (state) {
        case MAIN_MENU:
            DrawText("PYRAMID SOLITAIRE", 420, 180, 36, RAYWHITE);
            DrawText("ENTER -> Start Game", 450, 260, 20, RAYWHITE);
            DrawText("I -> Instructions", 460, 300, 20, RAYWHITE);
            DrawText("ESC -> Exit", 480, 340, 20, RAYWHITE);
            break;

        case INSTRUCTIONS:
            DrawText("INSTRUCTIONS", 470, 180, 32, RAYWHITE);
            DrawText("Remove cards whose sum is 13", 350, 250, 20, RAYWHITE);
            DrawText("King can be removed alone", 380, 290, 20, RAYWHITE);
            DrawText("BACKSPACE -> Menu", 420, 340, 20, RAYWHITE);
            break;

        case PLAYING:
            drawHUD();
            drawPyramidPlaceholder();
            drawStockPlaceholder();
            break;

        case GAME_OVER:
            DrawText(gameWon ? "YOU WIN!" : "GAME OVER",
                470, 200, 36,
                gameWon ? GREEN : RED);
            DrawText(TextFormat("Final Score: %d", score),
                450, 260, 20, RAYWHITE);
            DrawText("R - Restart | M - Menu",
                420, 310, 20, RAYWHITE);
            break;
        }
    }

    void drawHUD() {
        int timeGap = difftime(time(0), startTime);
        DrawText(TextFormat("PYRAMID SOLITARE GAME"), 400, 20, 30, YELLOW);
        DrawText(TextFormat("Score: %d", score), 20, 60, 20, RAYWHITE);
        DrawText(TextFormat("Time: %d", timeGap), 20, 90, 20, RAYWHITE);
    }

    void drawPyramidPlaceholder() {
        int startX = 400;
        int startY = 120;
        int cardW = 50;
        int cardH = 70;

        Card* curr = deck.head;
        for (int row = 0; row < 7; row++) {
            int x = startX - row * (cardW / 2);
            for (int col = 0; col <= row; col++) {
                if (!curr) 
                    break;

                DrawRectangle(x, startY + row * 80,
                    cardW, cardH,
                    curr->removed ? DARKGRAY : RAYWHITE);

                DrawText(TextFormat("%d", curr->value),
                    x + 18, startY + row * 80 + 25,
                    20, BLACK);

                curr = curr->next;
                x += cardW + 5;
            }
        }
    }

    void drawStockPlaceholder() {
        DrawRectangle(80, 200, 60, 80, BROWN);
        DrawText("STOCK", 85, 290, 18, RAYWHITE);
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
        else if (state == GAME_OVER) {
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