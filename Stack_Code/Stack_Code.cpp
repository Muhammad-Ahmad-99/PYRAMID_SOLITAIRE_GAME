#include "raylib.h"
#include <iostream>
#include <ctime>
#include <fstream>
#include <cstdlib>

using namespace std;

enum GameState {
    MAIN_MENU,
    PLAYING,
    INSTRUCTIONS,
    HIGH_SCORES
};

class Card
{
public:
    int value;
    int suit;
    bool faceUp;
    bool inPlay;

    Card()
    {
        value = 0;
        suit = 0;
        faceUp = false;
        inPlay = true;
    }

    Card(int v, int s)
    {
        value = v;
        suit = s;
        faceUp = false;
        inPlay = true;
    }
};

template <class T>
class StackNode
{
public:
    T data;
    StackNode<T>* next;

    StackNode(T d)
    {
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
    Stack()
    {
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

    T getAt(int index) {
        if (index < 0 || index >= size)
            return T();

        StackNode<T>* current = top;
        for (int i = 0; i < index; i++) {
            current = current->next;
        }
        return current->data;
    }

    bool contains(T data) {
        StackNode<T>* current = top;
        while (current) {
            if (current->data == data)
                return true;
            current = current->next;
        }
        return false;
    }

    void remove(T data) {
        if (!top)
            return;

        if (top->data == data) {
            pop();
            return;
        }

        StackNode<T>* current = top;
        while (current->next) {
            if (current->next->data == data) {
                StackNode<T>* temp = current->next;
                current->next = temp->next;
                delete temp;
                size--;
                return;
            }
            current = current->next;
        }
    }
};

class PyramidNode
{
public:
    Card* card;
    PyramidNode* left;
    PyramidNode* right;
    PyramidNode* nextInRow;
    int row;
    int col;
    bool blocked;

    PyramidNode(Card* c, int r, int cl)
    {
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
    int stockPosition;

    int score;
    int moves;
    int currentGameScoreIndex;
    bool isNewGame;
    float gameTime;
    bool gameWon;
    bool gameLost;

    Texture2D stockTexture;
    Texture2D background;
    Texture2D cardTextures[4][13];

    GameState currentState;
    bool isPaused;

    int highScores[5];
    int highScoreCount;

    const char* SCORE_FILE = "scores.txt";
    const char* SAVE_FILE = "savegame.dat";

    const int CARD_WIDTH = 90;
    const int CARD_HEIGHT = 130;
    const int CARD_SPACING = 20;

    Card allCards[52];
    int cardCount;
    Rectangle stockRect;

    struct SaveData {
        int score;
        int moves;
        float gameTime;
        bool gameWon;
        bool gameLost;

        int cardValues[52];
        int cardSuits[52];
        bool cardInPlay[52];
        bool cardFaceUp[52];

        int pyramidCardIndices[28];
        int stockCardIndices[24];
        int stockSize;
        int wasteCardIndices[24];
        int wasteHistorySize;
        int currentWasteIndex;
    };

    Sound cardSelectSound;
    Sound cardMatchSound;
    Sound cardMismatchSound;
    Sound stockDrawSound;
    float soundVolume;

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
        currentGameScoreIndex = -1;
        isNewGame = true;
        gameTime = 0.0f;
        gameWon = false;
        gameLost = false;
        cardCount = 0;
        stockPosition = 0;
        currentState = MAIN_MENU;
        isPaused = false;
        highScoreCount = 0;

        loadHighScores();
        InitAudioDevice();

        soundVolume = 0.7f;
        loadAllSounds();
        loadCardTextures();
        stockRect = { 0, 0, 0, 0 };
    }

    ~PyramidSolitaire() {
        if (currentState == PLAYING && !gameWon && !gameLost && score > 0) {
            saveCurrentGameScore();
            saveGame();
        }

        for (int s = 0; s < 4; s++) {
            for (int v = 0; v < 13; v++) {
                UnloadTexture(cardTextures[s][v]);
            }
        }
        UnloadTexture(background);
        UnloadTexture(stockTexture);

        if (cardSelectSound.frameCount > 0)
            UnloadSound(cardSelectSound);
        if (cardMatchSound.frameCount > 0)
            UnloadSound(cardMatchSound);
        if (cardMismatchSound.frameCount > 0)
            UnloadSound(cardMismatchSound);
        if (stockDrawSound.frameCount > 0)
            UnloadSound(stockDrawSound);

        CloseAudioDevice();
        clearPyramid();
    }

    void loadAllSounds() {
        cardSelectSound = { 0 };
        cardMatchSound = { 0 };
        cardMismatchSound = { 0 };
        stockDrawSound = { 0 };

        if (FileExists("sounds/card_select.mp3"))
            cardSelectSound = LoadSound("sounds/card_select.mp3");
        if (FileExists("sounds/card_match.mp3"))
            cardMatchSound = LoadSound("sounds/card_match.mp3");
        if (FileExists("sounds/card_mismatch.wav"))
            cardMismatchSound = LoadSound("sounds/card_mismatch.wav");
        if (FileExists("sounds/stock_draw.wav"))
            stockDrawSound = LoadSound("sounds/stock_draw.wav");

        setSoundVolume(soundVolume);
    }

    void setSoundVolume(float volume) {
        soundVolume = volume;
        if (cardSelectSound.frameCount > 0)
            SetSoundVolume(cardSelectSound, volume);
        if (cardMatchSound.frameCount > 0)
            SetSoundVolume(cardMatchSound, volume);
        if (cardMismatchSound.frameCount > 0)
            SetSoundVolume(cardMismatchSound, volume);
        if (stockDrawSound.frameCount > 0)
            SetSoundVolume(stockDrawSound, volume);
    }

    void playCardSelectSound() {
        if (cardSelectSound.frameCount > 0)
            PlaySound(cardSelectSound);
    }

    void playCardMatchSound() {
        if (cardMatchSound.frameCount > 0)
            PlaySound(cardMatchSound);
    }

    void playCardMismatchSound() {
        if (cardMismatchSound.frameCount > 0)
            PlaySound(cardMismatchSound);
    }

    void playStockDrawSound() {
        if (stockDrawSound.frameCount > 0)
            PlaySound(stockDrawSound);
    }

    void loadHighScores() {
        ifstream file(SCORE_FILE);
        highScoreCount = 0;

        if (!file.is_open())
            return;

        while (file >> highScores[highScoreCount] && highScoreCount < 5) {
            highScoreCount++;
        }
        file.close();

        for (int i = 0; i < highScoreCount - 1; i++) {
            for (int j = i + 1; j < highScoreCount; j++) {
                if (highScores[j] > highScores[i]) {
                    swap(highScores[i], highScores[j]);
                }
            }
        }
    }

    void saveHighScores() {
        ofstream file(SCORE_FILE, ios::trunc);
        for (int i = 0; i < highScoreCount; i++) {
            file << highScores[i] << endl;
        }
        file.close();
    }

    void updateHighScores(int newScore) {
        if (newScore <= 0)
            return;

        if (highScoreCount < 5) {
            highScores[highScoreCount++] = newScore;
        }
        else {
            highScores[4] = newScore;
        }

        for (int i = 0; i < highScoreCount - 1; i++) {
            for (int j = i + 1; j < highScoreCount; j++) {
                if (highScores[j] > highScores[i]) {
                    swap(highScores[i], highScores[j]);
                }
            }
        }

        saveHighScores();
    }

    void saveCurrentGameScore() {
        if (isNewGame) {
            if (highScoreCount < 5) {
                currentGameScoreIndex = highScoreCount;
                highScores[highScoreCount++] = score;
            }
            else {
                if (score > highScores[4]) {
                    currentGameScoreIndex = 4;
                    highScores[4] = score;
                }
                else {
                    currentGameScoreIndex = -1;
                }
            }
            isNewGame = false;
        }
        else if (currentGameScoreIndex >= 0) {
            highScores[currentGameScoreIndex] = score;
        }
        else if (score > 0 && highScoreCount < 5) {
            currentGameScoreIndex = highScoreCount;
            highScores[highScoreCount++] = score;
            isNewGame = false;
        }
        else if (score > 0 && highScoreCount == 5 && score > highScores[4]) {
            currentGameScoreIndex = 4;
            highScores[4] = score;
            isNewGame = false;
        }

        for (int i = 0; i < highScoreCount - 1; i++) {
            for (int j = i + 1; j < highScoreCount; j++) {
                if (highScores[j] > highScores[i]) {
                    swap(highScores[i], highScores[j]);
                }
            }
        }

        if (currentGameScoreIndex >= 0 && !isNewGame) {
            for (int i = 0; i < highScoreCount; i++) {
                if (highScores[i] == score) {
                    currentGameScoreIndex = i;
                    break;
                }
            }
        }

        saveHighScores();
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
        stockPosition = 0;

        isNewGame = true;
        currentGameScoreIndex = -1;

        createDeck();
        shuffleDeck();
        createPyramid();

        for (int i = 51; i >= 28; i--) {
            stock.push(&allCards[i]);
            stockBackup.push(&allCards[i]);
        }

        currentState = PLAYING;
        deleteSaveGame();
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
        playStockDrawSound();
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

            stockPosition = 0;
        }

        Card* card = stock.pop();
        card->faceUp = true;
        currentWasteCard = card;
        wasteHistory.push(card);
        stockPosition++;
    }

    void removeCards() {
        if (selectedCard1 && isKing(selectedCard1)) {
            playCardMatchSound();
            selectedCard1->inPlay = false;

            if (currentWasteCard == selectedCard1) {
                wasteHistory.remove(currentWasteCard);
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
            playCardMatchSound();
            selectedCard1->inPlay = false;
            selectedCard2->inPlay = false;

            if (currentWasteCard == selectedCard1 || currentWasteCard == selectedCard2) {
                if (currentWasteCard == selectedCard1) {
                    wasteHistory.remove(selectedCard1);
                }
                if (currentWasteCard == selectedCard2) {
                    wasteHistory.remove(selectedCard2);
                }

                if (wasteHistory.isEmpty())
                    currentWasteCard = NULL;
                else
                    currentWasteCard = wasteHistory.peek();
            }

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
            playCardMismatchSound();
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

        if (node && !isCardFree(node))
            return;

        playCardSelectSound();

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
            saveCurrentGameScore();
            deleteSaveGame();
            isNewGame = true;
            currentGameScoreIndex = -1;
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
        saveCurrentGameScore();
        deleteSaveGame();
        isNewGame = true;
        currentGameScoreIndex = -1;
    }

    void handleMouseClick(int mouseX, int mouseY) {
        if (gameWon || gameLost)
            return;

        for (int row = 0; row < 7; row++) {
            PyramidNode* current = pyramidRows[row];
            while (current) {
                if (current->card && current->card->inPlay) {
                    Rectangle cardRect = getPyramidCardRect(row, current->col);
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
            selectedCard1 = nullptr;
            selectedCard2 = nullptr;
            selectedNode1 = nullptr;
            selectedNode2 = nullptr;
            return;
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

            const char* suitSymbols[4] = { "♥", "♦", "♣", "♠" };
            DrawText(suitSymbols[card->suit], rect.x + 10, rect.y + 35, 25, suitColor);
        }

        if (selected) {
            DrawRectangleLinesEx(rect, 3, YELLOW);
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

    void renderHighScores() {
        BeginDrawing();
        drawBackground();

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });

        DrawText("TOP 5 HIGH SCORES", sw / 2 - 180, 100, 40, GOLD);

        for (int i = 0; i < highScoreCount; i++) {
            DrawText(TextFormat("%d. %d", i + 1, highScores[i]), sw / 2 - 50, 200 + i * 60, 35, WHITE);
        }

        if (highScoreCount == 0) {
            DrawText("No scores yet!", sw / 2 - 80, 200, 30, WHITE);
        }

        Rectangle backBtn = { (float)(sw / 2 - 100), sh - 120, 200, 50 };
        DrawRectangleRec(backBtn, DARKGRAY);
        DrawRectangleLinesEx(backBtn, 2, WHITE);
        DrawText("BACK TO MENU", sw / 2 - 85, sh - 105, 20, WHITE);

        EndDrawing();
    }

    void handleHighScoresClick(int mouseX, int mouseY) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        Rectangle backBtn = { (float)(sw / 2 - 100), sh - 120, 200, 50 };
        if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, backBtn)) {
            currentState = MAIN_MENU;
        }
    }

    void renderMainMenu() {
        BeginDrawing();
        drawBackground();

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        DrawText("PYRAMID SOLITAIRE", sw / 2 - 250, sh / 2 - 250, 50, GOLD);

        Rectangle playBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 130), 300, 60 };
        Rectangle loadBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 40), 300, 60 };
        Rectangle instructBtn = { (float)(sw / 2 - 150), (float)(sh / 2 + 50), 300, 60 };
        Rectangle highScoreBtn = { (float)(sw / 2 - 150), (float)(sh / 2 + 140), 300, 60 };
        Rectangle exitBtn = { (float)(sw / 2 - 150), (float)(sh / 2 + 230), 300, 60 };

        DrawRectangleRec(playBtn, DARKGREEN);
        DrawRectangleLinesEx(playBtn, 3, GREEN);
        DrawText("NEW GAME", sw / 2 - 130, sh / 2 - 110, 25, WHITE);

        DrawRectangleRec(loadBtn, MAROON);
        DrawRectangleLinesEx(loadBtn, 3, RED);
        DrawText("RESUME GAME", sw / 2 - 90, sh / 2 - 20, 25, WHITE);

        DrawRectangleRec(instructBtn, DARKBLUE);
        DrawRectangleLinesEx(instructBtn, 3, BLUE);
        DrawText("INSTRUCTIONS", sw / 2 - 110, sh / 2 + 70, 25, WHITE);

        DrawRectangleRec(highScoreBtn, ORANGE);
        DrawRectangleLinesEx(highScoreBtn, 3, RED);
        DrawText("HIGH SCORES", sw / 2 - 100, sh / 2 + 160, 25, WHITE);

        DrawRectangleRec(exitBtn, DARKGRAY);
        DrawRectangleLinesEx(exitBtn, 3, BLACK);
        DrawText("EXIT GAME", sw / 2 - 80, sh / 2 + 250, 25, WHITE);

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

        DrawText("CONTROLS:", sw / 2 - 350, y, 25, YELLOW);
        y += spacing;
        DrawText("P - Pause/Resume game", sw / 2 - 350, y, 20, WHITE);
        y += spacing;
        DrawText("BACKSPACE - Return to main menu", sw / 2 - 350, y, 20, WHITE);
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

    int main() {
        const int screenWidth = 1400;
        const int screenHeight = 950;

        InitWindow(screenWidth, screenHeight, "Pyramid Solitaire Game - STACK BASED");
        SetTargetFPS(60);

        PyramidSolitaire game;

        while (!WindowShouldClose()) {
            game.update(GetFrameTime());
            game.render();
        }

        CloseWindow();
        return 0;
    }
};
