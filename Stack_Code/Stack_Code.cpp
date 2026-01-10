#include "raylib.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <string>

using namespace std;

enum GameState {
    MAIN_MENU,
    PLAYING,
    INSTRUCTIONS,
    HIGH_SCORES
};

class Card {
public:
    int value;
    int suit;
    bool faceUp;
    bool inPlay;

    Card() {
        value = 0;
        suit = 0;
        faceUp = false;
        inPlay = true;
    }

    Card(int v, int s) {
        value = v;
        suit = s;
        faceUp = false;
        inPlay = true;
    }
};

template <class T>
class StackNode {
public:
    T data;
    StackNode<T>* next;

    StackNode(T d) {
        data = d;
        next = NULL;
    }
};

template<typename T>
class Stack {
private:
    StackNode<T>* top;
    int size;

public:
    Stack() {
        top = NULL;
        size = 0;
    }

    ~Stack() {
        clear();
    }

    void push(T data) {
        StackNode<T>* newNode = new StackNode<T>(data);
        newNode->next = top;
        top = newNode;
        size++;
    }

    T pop() {
        if (!top)
            return T();

        T data = top->data;
        StackNode<T>* temp = top;
        top = top->next;
        delete temp;
        size--;
        return data;
    }

    T peek() {
        if (top)
            return top->data;
        return T();
    }

    bool isEmpty() {
        return top == nullptr;
    }

    int getSize() {
        return size;
    }

    void clear() {
        while (top) {
            StackNode<T>* temp = top;
            top = top->next;
            delete temp;
        }
        size = 0;
    }

    StackNode<T>* getTop() {
        return top;
    }
};

class PyramidNode {
public:
    Card* card;
    PyramidNode* left;
    PyramidNode* right;
    PyramidNode* nextInRow;
    int row;
    int col;
    bool blocked;

    PyramidNode(Card* c, int r, int cl) {
        card = c;
        left = NULL;
        right = NULL;
        nextInRow = NULL;
        row = r;
        col = cl;
        blocked = true;
    }
};

class PyramidSolitaire {
private:
    Stack<Card> deck;
    Stack<Card*> stock;
    Stack<Card*> stockBackup;
    Stack<Card*> wasteHistory;

    PyramidNode* pyramidRows[7];

    Card* selectedCard1;
    Card* selectedCard2;
    PyramidNode* selectedNode1;
    PyramidNode* selectedNode2;

    Card* currentWasteCard;

    int score;
    int moves;
    float gameTime;
    bool gameWon;
    bool gameLost;

    GameState currentState;
    bool isPaused;

    Card allCards[52];
    int cardCount;

    // ALI'S PART: GUI Components
    Texture2D stockTexture;
    Texture2D background;
    Texture2D cardTextures[4][13];
    Rectangle stockRect;
    const int CARD_WIDTH = 90;
    const int CARD_HEIGHT = 130;
    const int CARD_SPACING = 20;

public:
    PyramidSolitaire() {
        for (int i = 0; i < 7; i++) {
            pyramidRows[i] = nullptr;
        }

        selectedCard1 = nullptr;
        selectedCard2 = nullptr;
        selectedNode1 = nullptr;
        selectedNode2 = nullptr;
        currentWasteCard = nullptr;
        score = 0;
        moves = 0;
        gameTime = 0.0f;
        gameWon = false;
        gameLost = false;
        cardCount = 0;
        currentState = MAIN_MENU;
        isPaused = false;

        loadCardTextures(); // ALI'S PART
        stockRect = { 0, 0, 0, 0 };
    }

    ~PyramidSolitaire() {
        clearPyramid();
        // ALI'S PART: Cleanup textures
        for (int s = 0; s < 4; s++) {
            for (int v = 0; v < 13; v++) {
                UnloadTexture(cardTextures[s][v]);
            }
        }
        UnloadTexture(background);
        UnloadTexture(stockTexture);
    }

    // ALI'S PART: Load textures
    void loadCardTextures() {
        const char* suits[4] = { "H", "D", "C", "S" };
        const char* values[13] = { "A","2","3","4","5","6","7","8","9","10","J","Q","K" };

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

    void clearPyramid() {
        for (int row = 0; row < 7; row++) {
            PyramidNode* current = pyramidRows[row];
            while (current) {
                PyramidNode* next = current->nextInRow;
                delete current;
                current = next;
            }
            pyramidRows[row] = nullptr;
        }
    }

    void initGame() {
        clearPyramid();
        deck.clear();
        stock.clear();
        stockBackup.clear();
        wasteHistory.clear();

        selectedCard1 = nullptr;
        selectedCard2 = nullptr;
        selectedNode1 = nullptr;
        selectedNode2 = nullptr;
        currentWasteCard = nullptr;
        score = 0;
        moves = 0;
        gameTime = 0.0f;
        gameWon = false;
        gameLost = false;
        cardCount = 0;

        createDeck();
        shuffleDeck();
        createPyramid();

        for (int i = 51; i >= 28; i--) {
            stock.push(&allCards[i]);
            stockBackup.push(&allCards[i]);
        }

        currentState = PLAYING;
    }

    void createDeck() {
        cardCount = 0;
        for (int suit = 0; suit < 4; suit++) {
            for (int value = 1; value <= 13; value++) {
                allCards[cardCount] = Card(value, suit);
                deck.push(allCards[cardCount]);
                cardCount++;
            }
        }
    }

    void shuffleDeck() {
        srand(time(nullptr));
        Card tempDeck[52];
        int index = 0;

        Stack<Card> tempStack;
        while (!deck.isEmpty()) {
            tempStack.push(deck.pop());
        }

        while (!tempStack.isEmpty()) {
            tempDeck[index++] = tempStack.pop();
        }

        for (int i = 51; i > 0; i--) {
            int j = rand() % (i + 1);
            swap(tempDeck[i], tempDeck[j]);
        }

        for (int i = 0; i < 52; i++) {
            allCards[i] = tempDeck[i];
        }

        deck.clear();
        for (int i = 0; i < 52; i++) {
            deck.push(allCards[i]);
        }
    }

    void createPyramid() {
        int cardIndex = 0;
        PyramidNode* prevRowHeads[7] = { nullptr };

        for (int row = 0; row < 7; row++) {
            PyramidNode* rowHead = nullptr;
            PyramidNode* rowTail = nullptr;

            for (int col = 0; col <= row; col++) {
                Card* card = &allCards[cardIndex++];
                card->faceUp = true;
                PyramidNode* node = new PyramidNode(card, row, col);

                if (!rowHead) {
                    rowHead = node;
                    rowTail = node;
                }
                else {
                    rowTail->nextInRow = node;
                    rowTail = node;
                }

                if (row > 0) {
                    PyramidNode* prevRowNode = prevRowHeads[row - 1];
                    int count = 0;
                    while (prevRowNode && count < col) {
                        prevRowNode = prevRowNode->nextInRow;
                        count++;
                    }

                    if (prevRowNode && count == col) {
                        if (col < row) {
                            prevRowNode->left = node;
                        }
                        if (col > 0) {
                            PyramidNode* leftParent = prevRowHeads[row - 1];
                            count = 0;
                            while (leftParent && count < col - 1) {
                                leftParent = leftParent->nextInRow;
                                count++;
                            }
                            if (leftParent) {
                                leftParent->right = node;
                            }
                        }
                    }
                }

                if (row == 6) {
                    node->blocked = false;
                }
            }

            pyramidRows[row] = rowHead;
            prevRowHeads[row] = rowHead;
        }
    }

    void updateBlockedStatus() {
        for (int row = 0; row < 7; row++) {
            PyramidNode* current = pyramidRows[row];
            while (current) {
                if (current->card && current->card->inPlay) {
                    bool leftBlocking = (current->left && current->left->card && current->left->card->inPlay);
                    bool rightBlocking = (current->right && current->right->card && current->right->card->inPlay);
                    current->blocked = leftBlocking || rightBlocking;
                }
                current = current->nextInRow;
            }
        }
    }

    bool isCardFree(PyramidNode* node) {
        if (!node || !node->card || !node->card->inPlay)
            return false;
        return !node->blocked;
    }

    bool isValidMove(Card* c1, Card* c2) {
        if (!c1 || !c2)
            return false;
        if (!c1->inPlay || !c2->inPlay)
            return false;
        return (c1->value + c2->value) == 13;
    }

    bool isKing(Card* c) {
        if (!c)
            return false;
        return c->value == 13;
    }

    void drawCardFromStock() {
        moves++;

        if (stock.isEmpty()) {
            Stack<Card*> tempStack;
            StackNode<Card*>* backupNode = stockBackup.getTop();
            while (backupNode) {
                if (backupNode->data->inPlay) {
                    tempStack.push(backupNode->data);
                }
                backupNode = backupNode->next;
            }

            stock.clear();
            while (!tempStack.isEmpty()) {
                stock.push(tempStack.pop());
            }
        }

        if (!stock.isEmpty()) {
            Card* card = stock.pop();
            card->faceUp = true;
            currentWasteCard = card;
            wasteHistory.push(card);
        }
    }

    void removeCards() {
        if (selectedCard1 && isKing(selectedCard1)) {
            selectedCard1->inPlay = false;

            if (currentWasteCard == selectedCard1) {
                wasteHistory.pop();
                if (wasteHistory.isEmpty()) {
                    currentWasteCard = NULL;
                }
                else {
                    currentWasteCard = wasteHistory.peek();
                }
            }

            score += 10;
            moves++;
            selectedCard1 = nullptr;
            selectedNode1 = nullptr;
            updateBlockedStatus();
            checkWinCondition();
            return;
        }

        if (selectedCard1 && selectedCard2 && isValidMove(selectedCard1, selectedCard2)) {
            selectedCard1->inPlay = false;
            selectedCard2->inPlay = false;

            score += 20;
            moves++;

            selectedCard1 = nullptr;
            selectedCard2 = nullptr;
            selectedNode1 = nullptr;
            selectedNode2 = nullptr;

            updateBlockedStatus();
            checkWinCondition();
        }
        else if (selectedCard1 && selectedCard2) {
            moves++;
            selectedCard1 = nullptr;
            selectedCard2 = nullptr;
            selectedNode1 = nullptr;
            selectedNode2 = nullptr;
        }
    }

    void selectCard(Card* card, PyramidNode* node) {
        if (!card || !card->inPlay)
            return;

        if (node && !isCardFree(node)) {
            return;
        }

        if (isKing(card)) {
            selectedCard1 = card;
            selectedNode1 = node;
            selectedCard2 = nullptr;
            selectedNode2 = nullptr;
            removeCards();
            return;
        }

        if (!selectedCard1) {
            selectedCard1 = card;
            selectedNode1 = node;
        }
        else if (selectedCard1 == card) {
            selectedCard1 = nullptr;
            selectedNode1 = nullptr;
        }
        else {
            selectedCard2 = card;
            selectedNode2 = node;
            removeCards();
        }
    }

    void checkWinCondition() {
        bool allRemoved = true;
        for (int row = 0; row < 7; row++) {
            PyramidNode* current = pyramidRows[row];
            while (current) {
                if (current->card && current->card->inPlay) {
                    allRemoved = false;
                    break;
                }
                current = current->nextInRow;
            }
            if (!allRemoved)
                break;
        }

        if (allRemoved) {
            gameWon = true;
        }
    }

    void checkLoseCondition() {
        Stack<Card*> freeCards;

        for (int row = 0; row < 7; row++) {
            PyramidNode* current = pyramidRows[row];
            while (current) {
                if (isCardFree(current)) {
                    freeCards.push(current->card);
                }
                current = current->nextInRow;
            }
        }

        if (currentWasteCard && currentWasteCard->inPlay) {
            freeCards.push(currentWasteCard);
        }

        StackNode<Card*>* checkNode = freeCards.getTop();
        while (checkNode) {
            if (isKing(checkNode->data))
                return;
            checkNode = checkNode->next;
        }

        StackNode<Card*>* node1 = freeCards.getTop();
        while (node1) {
            StackNode<Card*>* node2 = node1->next;
            while (node2) {
                if (isValidMove(node1->data, node2->data)) {
                    return;
                }
                node2 = node2->next;
            }
            node1 = node1->next;
        }

        if (!stock.isEmpty())
            return;

        StackNode<Card*>* backupCheckNode = stockBackup.getTop();
        while (backupCheckNode) {
            if (backupCheckNode->data->inPlay)
                return;
            backupCheckNode = backupCheckNode->next;
        }

        gameLost = true;
    }

    // ALI'S PART: Get card position
    Rectangle getPyramidCardRect(int row, int col) {
        int startX = (GetScreenWidth() / 2) - (row * (CARD_WIDTH + CARD_SPACING) / 2);
        int x = startX + col * (CARD_WIDTH + CARD_SPACING);
        int y = 100 + row * (CARD_HEIGHT / 2 + CARD_SPACING);
        return { (float)x, (float)y, (float)CARD_WIDTH, (float)CARD_HEIGHT };
    }

    // ALI'S PART: Draw card
    void drawCard(Card* card, Rectangle rect, bool selected) {
        if (!card) return;

        Texture2D tex = cardTextures[card->suit][card->value - 1];
        if (tex.id != 0) {
            DrawTexturePro(tex, { 0, 0, (float)tex.width, (float)tex.height }, rect, { 0, 0 }, 0, WHITE);
        }
        else {
            DrawRectangleRec(rect, WHITE);
            DrawRectangleLinesEx(rect, 2, BLACK);
        }

        if (selected) DrawRectangleLinesEx(rect, 3, YELLOW);
    }

    // ALI'S PART: Draw background
    void drawBackground() {
        if (background.id != 0) {
            DrawTexturePro(background, { 0, 0, (float)background.width, (float)background.height },
                { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, { 0, 0 }, 0, WHITE);
        }
        else {
            ClearBackground(DARKGREEN);
        }
    }

    // ALI'S PART: Render main menu
    void renderMainMenu() {
        BeginDrawing();
        drawBackground();
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        DrawText("PYRAMID SOLITAIRE", sw / 2 - 250, sh / 2 - 300, 50, GOLD);

        Rectangle playBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 180), 300, 60 };
        Rectangle instructBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 90), 300, 60 };
        Rectangle exitBtn = { (float)(sw / 2 - 150), (float)(sh / 2), 300, 60 };

        DrawRectangleRec(playBtn, DARKGREEN);
        DrawRectangleLinesEx(playBtn, 3, GREEN);
        DrawText("NEW GAME", sw / 2 - 110, sh / 2 - 160, 25, WHITE);

        DrawRectangleRec(instructBtn, DARKBLUE);
        DrawRectangleLinesEx(instructBtn, 3, BLUE);
        DrawText("INSTRUCTIONS", sw / 2 - 110, sh / 2 - 70, 25, WHITE);

        DrawRectangleRec(exitBtn, DARKGRAY);
        DrawRectangleLinesEx(exitBtn, 3, BLACK);
        DrawText("EXIT GAME", sw / 2 - 80, sh / 2 + 20, 25, WHITE);

        EndDrawing();
    }

    // ALI'S PART: Render instructions
    void renderInstructions() {
        BeginDrawing();
        drawBackground();
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        DrawText("HOW TO PLAY", sw / 2 - 150, 150, 40, GOLD);
        DrawText("Pair cards that sum to 13", sw / 2 - 200, 250, 25, WHITE);
        DrawText("Kings removed alone", sw / 2 - 150, 300, 25, WHITE);

        Rectangle backBtn = { (float)(sw / 2 - 100), (float)(sh - 120), 200, 50 };
        DrawRectangleRec(backBtn, DARKGRAY);
        DrawText("BACK", sw / 2 - 40, sh - 105, 20, WHITE);

        EndDrawing();
    }

    // ALI'S PART: Main render
    void render() {
        if (currentState == MAIN_MENU) { renderMainMenu(); return; }
        if (currentState == INSTRUCTIONS) { renderInstructions(); return; }

        BeginDrawing();
        drawBackground();
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        DrawText(TextFormat("Score: %d", score), 20, 20, 25, GOLD);
        DrawText(TextFormat("Moves: %d", moves), sw - 150, 20, 25, YELLOW);

        for (int row = 0; row < 7; row++) {
            PyramidNode* current = pyramidRows[row];
            while (current) {
                if (current->card && current->card->inPlay) {
                    Rectangle rect = getPyramidCardRect(current->row, current->col);
                    bool selected = (current == selectedNode1 || current == selectedNode2);
                    drawCard(current->card, rect, selected);
                    if (current->blocked) DrawRectangle(rect.x, rect.y, rect.width, 5, RED);
                }
                current = current->nextInRow;
            }
        }

        int uiStartY = 150 + 7 * (CARD_HEIGHT / 2 + CARD_SPACING);

        DrawText("WASTE", 50, uiStartY - 30, 20, WHITE);
        if (currentWasteCard && currentWasteCard->inPlay) {
            Rectangle wasteRect = { 50, (float)uiStartY, CARD_WIDTH, CARD_HEIGHT };
            bool selected = (currentWasteCard == selectedCard1 || currentWasteCard == selectedCard2);
            drawCard(currentWasteCard, wasteRect, selected);
        }

        stockRect = { 180.0f, (float)uiStartY, (float)CARD_WIDTH, (float)CARD_HEIGHT };
        DrawText("STOCK", 180, uiStartY - 30, 20, WHITE);
        if (stockTexture.id != 0 && !stock.isEmpty()) {
            DrawTexturePro(stockTexture, { 0, 0, (float)stockTexture.width, (float)stockTexture.height },
                stockRect, { 0, 0 }, 0, WHITE);
        }

        DrawText(TextFormat("Time: %02d:%02d:%02d", (int)gameTime / 3600, ((int)gameTime % 3600) / 60, (int)gameTime % 60),
            sw / 2 - 80, sh - 30, 25, WHITE);

        Rectangle restartBtn = { (float)(sw - 150), (float)(sh - 60), 120, 50 };
        DrawRectangleRec(restartBtn, MAROON);
        DrawText("RESTART", sw - 140, sh - 45, 20, WHITE);

        if (gameWon) {
            DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });
            DrawText("YOU WIN!", sw / 2 - 100, sh / 2, 40, GOLD);
        }
        else if (gameLost) {
            DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });
            DrawText("NO MOVES LEFT!", sw / 2 - 150, sh / 2, 40, RED);
        }

        if (isPaused) {
            DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });
            DrawText("PAUSED", sw / 2 - 80, sh / 2, 40, YELLOW);
        }

        EndDrawing();
    }

    // ALI'S PART: Handle clicks
    void handleMouseClick(int mouseX, int mouseY) {
        if (gameWon || gameLost) { currentState = MAIN_MENU; return; }

        for (int row = 0; row < 7; row++) {
            PyramidNode* current = pyramidRows[row];
            while (current) {
                if (current->card && current->card->inPlay) {
                    Rectangle cardRect = getPyramidCardRect(current->row, current->col);
                    if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, cardRect)) {
                        selectCard(current->card, current);
                        return;
                    }
                }
                current = current->nextInRow;
            }
        }

        if (currentWasteCard && currentWasteCard->inPlay) {
            int uiStartY = 150 + 7 * (CARD_HEIGHT / 2 + CARD_SPACING);
            Rectangle wasteRect = { 50, (float)uiStartY, CARD_WIDTH, CARD_HEIGHT };
            if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, wasteRect)) {
                selectCard(currentWasteCard, nullptr);
                return;
            }
        }

        if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, stockRect)) {
            drawCardFromStock();
        }
    }

    void handleMainMenuClick(int mouseX, int mouseY) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        Rectangle playBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 180), 300, 60 };
        Rectangle instructBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 90), 300, 60 };
        Rectangle exitBtn = { (float)(sw / 2 - 150), (float)(sh / 2), 300, 60 };

        if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, playBtn)) {
            initGame();
        }
        else if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, instructBtn)) {
            currentState = INSTRUCTIONS;
        }
        else if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, exitBtn)) {
            CloseWindow();
            exit(0);
        }
    }

    void handleInstructionsClick(int mouseX, int mouseY) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        Rectangle backBtn = { (float)(sw / 2 - 100), (float)(sh - 120), 200, 50 };
        if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, backBtn)) {
            currentState = MAIN_MENU;
        }
    }

    void update(float deltaTime) {
        if (IsKeyPressed(KEY_P) && !gameWon && !gameLost) {
            isPaused = !isPaused;
        }

        if (isPaused) return;

        if (currentState == PLAYING && !gameWon && !gameLost) {
            gameTime += deltaTime;
            checkLoseCondition();
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();

            if (currentState == MAIN_MENU) {
                handleMainMenuClick((int)mousePos.x, (int)mousePos.y);
            }
            else if (currentState == INSTRUCTIONS) {
                handleInstructionsClick((int)mousePos.x, (int)mousePos.y);
            }
            else if (currentState == PLAYING) {
                int sw = GetScreenWidth();
                int sh = GetScreenHeight();
                Rectangle restartBtn = { (float)(sw - 150), (float)(sh - 60), 120, 50 };
                if (CheckCollisionPointRec(mousePos, restartBtn)) {
                    initGame();
                    return;
                }
                handleMouseClick((int)mousePos.x, (int)mousePos.y);
            }
        }
    }
};

int main() {
    const int screenWidth = 1400;
    const int screenHeight = 950;

    InitWindow(screenWidth, screenHeight, "Pyramid Solitaire - Stack Version");
    SetTargetFPS(60);

    PyramidSolitaire game;

    while (!WindowShouldClose()) {
        game.update(GetFrameTime());
        game.render();
    }

    CloseWindow();
    return 0;
}