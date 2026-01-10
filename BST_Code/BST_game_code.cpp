#include "raylib.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <string>
#include <fstream>
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
    int position;

    Card() {
        value = 0;
        suit = 0;
        faceUp = false;
        inPlay = true;
        position = 0;
    }

    Card(int v, int s, int pos = 0) {
        value = v;
        suit = s;
        faceUp = false;
        inPlay = true;
        position = pos;
    }

    bool operator<(const Card& other) const {
        return position < other.position;
    }

    bool operator>(const Card& other) const {
        return position > other.position;
    }

    bool operator==(const Card& other) const {
        return position == other.position;
    }
};

template <class T>
class BSTNode {
public:
    T data;
    BSTNode<T>* left;
    BSTNode<T>* right;

    BSTNode(T d) {
        data = d;
        left = NULL;
        right = NULL;
    }
};

template<typename T>
class BST {
private:
    BSTNode<T>* root;
    int size;

    BSTNode<T>* insertHelper(BSTNode<T>* node, T data) {
        if (node == NULL) {
            size++;
            return new BSTNode<T>(data);
        }
        if (data < node->data) {
            node->left = insertHelper(node->left, data);
        }
        else if (data > node->data) {
            node->right = insertHelper(node->right, data);
        }
        return node;
    }

    BSTNode<T>* searchHelper(BSTNode<T>* node, T data) {
        if (node == NULL || node->data == data) {
            return node;
        }
        if (data < node->data) {
            return searchHelper(node->left, data);
        }
        else {
            return searchHelper(node->right, data);
        }
    }

    void clearHelper(BSTNode<T>* node) {
        if (node == NULL) return;
        clearHelper(node->left);
        clearHelper(node->right);
        delete node;
    }

    BSTNode<T>* findMin(BSTNode<T>* node) {
        while (node && node->left != NULL) {
            node = node->left;
        }
        return node;
    }

    BSTNode<T>* findMax(BSTNode<T>* node) {
        while (node && node->right != NULL) {
            node = node->right;
        }
        return node;
    }

    BSTNode<T>* deleteHelper(BSTNode<T>* node, T data) {
        if (node == NULL)
            return NULL;
        if (data < node->data) {
            node->left = deleteHelper(node->left, data);
        }
        else if (data > node->data) {
            node->right = deleteHelper(node->right, data);
        }
        else {
            if (node->left == NULL) {
                BSTNode<T>* temp = node->right;
                delete node;
                size--;
                return temp;
            }
            else if (node->right == NULL) {
                BSTNode<T>* temp = node->left;
                delete node;
                size--;
                return temp;
            }
            BSTNode<T>* temp = findMin(node->right);
            node->data = temp->data;
            node->right = deleteHelper(node->right, temp->data);
        }
        return node;
    }

    void inorderHelper(BSTNode<T>* node, T* arr, int& index) {
        if (node == NULL) return;
        inorderHelper(node->left, arr, index);
        arr[index++] = node->data;
        inorderHelper(node->right, arr, index);
    }

public:
    BST() {
        root = NULL;
        size = 0;
    }

    ~BST() {
        clear();
    }

    void insert(T data) {
        root = insertHelper(root, data);
    }

    BSTNode<T>* search(T data) {
        return searchHelper(root, data);
    }

    void remove(T data) {
        root = deleteHelper(root, data);
    }

    bool isEmpty() {
        return root == NULL;
    }

    int getSize() {
        return size;
    }

    void clear() {
        clearHelper(root);
        root = NULL;
        size = 0;
    }

    BSTNode<T>* getRoot() {
        return root;
    }

    void toArray(T* arr) {
        int index = 0;
        inorderHelper(root, arr, index);
    }

    BSTNode<T>* getMax() {
        return findMax(root);
    }

    BSTNode<T>* getMin() {
        return findMin(root);
    }
};

struct PyramidCard {
    Card* card;
    int row;
    int col;
    bool blocked;
    int leftChildPos;
    int rightChildPos;
    PyramidCard() {
        card = nullptr;
        row = 0;
        col = 0;
        leftChildPos = -1;
        rightChildPos = -1;
        blocked = true;
    }

    PyramidCard(Card* c, int r, int cl) {
        card = c;
        row = r;
        col = cl;
        blocked = true;
        leftChildPos = (r + 1) * 100 + cl;
        rightChildPos = (r + 1) * 100 + (cl + 1);
        if (r == 6)
            blocked = false;
    }

    bool operator<(const PyramidCard& other) const {
        return (row * 100 + col) < (other.row * 100 + other.col);
    }

    bool operator>(const PyramidCard& other) const {
        return (row * 100 + col) > (other.row * 100 + other.col);
    }

    bool operator==(const PyramidCard& other) const {
        return (row * 100 + col) == (other.row * 100 + other.col);
    }
};

class PyramidSolitaire {
public:
    BST<PyramidCard> pyramidBST;
    Card allCards[52];
    PyramidCard allPyramidCards[28];
    Card* stockArray[52];
    int stockTop;
    Card* wasteArray[52];
    int wasteTop;
    Card* selectedCard1;
    Card* selectedCard2;
    PyramidCard* selectedPyramid1;
    PyramidCard* selectedPyramid2;
    Card* currentWasteCard;
    int score;
    int moves;
    float gameTime;
    bool gameWon;
    bool gameLost;
    bool savedGameExists;
    GameState currentState;
    bool isPaused;
    Texture2D stockTexture;
    Texture2D background;
    Texture2D cardTextures[4][13];
    Rectangle stockRect;
    const int CARD_WIDTH = 90;
    const int CARD_HEIGHT = 130;
    const int CARD_SPACING = 20;

    int highScores[5];
    int highScoreCount;
    int currentGameScoreIndex;
    bool isNewGame;
    bool showSaveMessage;
    float saveMessageTimer;
    const char* SCORE_FILE = "scores.txt";
    const char* SAVE_FILE = "gamesave.dat";

public:
    PyramidSolitaire() {
        selectedCard1 = selectedCard2 = nullptr;
        selectedPyramid1 = selectedPyramid2 = nullptr;
        currentWasteCard = nullptr;
        score = moves = 0;
        gameTime = 0;
        gameWon = gameLost = false;
        stockTop = wasteTop = -1;
        currentState = MAIN_MENU;
        isPaused = false;



        loadCardTextures();
        loadHighScores();
        saveHighScores();
        saveCurrentGameScore();

        stockRect = { 0, 0, 0, 0 };
        savedGameExists = false;
        checkSavedGame();
        highScoreCount = 0;
        currentGameScoreIndex = -1;
        isNewGame = true;
        showSaveMessage = false;
        saveMessageTimer = 0.0f;

    }

    ~PyramidSolitaire() {
        if (currentState == PLAYING && !gameWon && !gameLost && score > 0)
        {
            saveCurrentGameScore();
        }
        pyramidBST.clear();
        for (int s = 0; s < 4; s++) {
            for (int v = 0; v < 13; v++) {
                UnloadTexture(cardTextures[s][v]);
            }
        }
        UnloadTexture(background);
        UnloadTexture(stockTexture);
    }
    void checkSavedGame() {
        ifstream file(SAVE_FILE, ios::binary);
        if (file.is_open()) {
            savedGameExists = true;
            file.close();
        }
        else {
            savedGameExists = false;
        }
    }
    void saveGame() {
        ofstream file(SAVE_FILE, ios::binary | ios::trunc);
        if (!file.is_open()) {
            cout << "Error: Could not save game!" << endl;
            return;
        }
        file.write((char*)&score, sizeof(score));
        file.write((char*)&moves, sizeof(moves));
        file.write((char*)&gameTime, sizeof(gameTime));
        file.write((char*)&stockTop, sizeof(stockTop));
        file.write((char*)&wasteTop, sizeof(wasteTop));
        for (int i = 0; i < 52; i++) {
            file.write((char*)&allCards[i].value, sizeof(allCards[i].value));
            file.write((char*)&allCards[i].suit, sizeof(allCards[i].suit));
            file.write((char*)&allCards[i].faceUp, sizeof(allCards[i].faceUp));
            file.write((char*)&allCards[i].inPlay, sizeof(allCards[i].inPlay));
            file.write((char*)&allCards[i].position, sizeof(allCards[i].position));
        }
        for (int i = 0; i < 28; i++) {
            file.write((char*)&allPyramidCards[i].blocked, sizeof(allPyramidCards[i].blocked));
        }
        for (int i = 0; i <= stockTop; i++) {
            int pos = stockArray[i]->position;
            file.write((char*)&pos, sizeof(pos));
        }
        for (int i = 0; i <= wasteTop; i++) {
            int pos = wasteArray[i]->position;
            file.write((char*)&pos, sizeof(pos));
        }
        int wastePos = (currentWasteCard != NULL) ? currentWasteCard->position : -1;
        file.write((char*)&wastePos, sizeof(wastePos));

        file.close();

        savedGameExists = true;
        showSaveMessage = true;
        saveMessageTimer = 2.0f;
        cout << "Game saved successfully to " << SAVE_FILE << endl;
    }
    bool loadGame() {
        ifstream file(SAVE_FILE, ios::binary);
        if (!file.is_open()) {
            cout << "Error: Could not load game from " << SAVE_FILE << endl;
            return false;
        }

        pyramidBST.clear();
        stockTop = -1;
        wasteTop = -1;

        file.read((char*)&score, sizeof(score));
        file.read((char*)&moves, sizeof(moves));
        file.read((char*)&gameTime, sizeof(gameTime));
        file.read((char*)&stockTop, sizeof(stockTop));
        file.read((char*)&wasteTop, sizeof(wasteTop));

        for (int i = 0; i < 52; i++) {
            file.read((char*)&allCards[i].value, sizeof(allCards[i].value));
            file.read((char*)&allCards[i].suit, sizeof(allCards[i].suit));
            file.read((char*)&allCards[i].faceUp, sizeof(allCards[i].faceUp));
            file.read((char*)&allCards[i].inPlay, sizeof(allCards[i].inPlay));
            file.read((char*)&allCards[i].position, sizeof(allCards[i].position));
        }

        for (int i = 0; i < 28; i++) {
            file.read((char*)&allPyramidCards[i].blocked, sizeof(allPyramidCards[i].blocked));
        }

        int cardIdx = 0;
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col <= row; col++) {

                allPyramidCards[cardIdx].card = &allCards[cardIdx];
                allPyramidCards[cardIdx].row = row;
                allPyramidCards[cardIdx].col = col;
                allPyramidCards[cardIdx].leftChildPos = (row + 1) * 100 + col;
                allPyramidCards[cardIdx].rightChildPos = (row + 1) * 100 + (col + 1);

                pyramidBST.insert(allPyramidCards[cardIdx]);
                cardIdx++;
            }
        }

        for (int i = 0; i <= stockTop; i++) {
            int pos;
            file.read((char*)&pos, sizeof(pos));
            stockArray[i] = &allCards[pos];
        }

        for (int i = 0; i <= wasteTop; i++) {
            int pos;
            file.read((char*)&pos, sizeof(pos));
            wasteArray[i] = &allCards[pos];
        }

        int wastePos;
        file.read((char*)&wastePos, sizeof(wastePos));
        currentWasteCard = (wastePos >= 0) ? &allCards[wastePos] : NULL;

        file.close();

        selectedCard1 = nullptr;
        selectedCard2 = nullptr;
        selectedPyramid1 = nullptr;
        selectedPyramid2 = nullptr;
        gameWon = false;
        gameLost = false;
        isNewGame = false;
        cardCount = 52;

        currentState = PLAYING;
        cout << "Game loaded successfully from " << SAVE_FILE << endl;
        return true;
    }
    void deleteSavedGame() {
        if (remove(SAVE_FILE) == 0) {
            cout << "Saved game deleted." << endl;
        }
        savedGameExists = false;
    }

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

    void createDeck() {
        int index = 0;
        for (int suit = 0; suit < 4; suit++) {
            for (int value = 1; value <= 13; value++) {
                allCards[index] = Card(value, suit, index);
                index++;
            }
        }
    }

    void shuffleDeck() {
        srand(time(nullptr));
        for (int i = 51; i > 0; i--) {
            int j = rand() % (i + 1);
            swap(allCards[i], allCards[j]);
            allCards[i].position = i;
        }
    }

    void initGame() {
        pyramidBST.clear();
        score = moves = 0;
        gameTime = 0;
        gameWon = gameLost = false;
        stockTop = wasteTop = -1;
        selectedCard1 = selectedCard2 = nullptr;
        selectedPyramid1 = selectedPyramid2 = nullptr;
        currentWasteCard = nullptr;
        createDeck();
        shuffleDeck();
        createPyramid();
        int cardIndex = 0;
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col <= row; col++) {
                allCards[cardIndex].faceUp = true;
                allPyramidCards[cardIndex] = PyramidCard(&allCards[cardIndex], row, col);
                pyramidBST.insert(allPyramidCards[cardIndex]);
                cardIndex++;
            }
        }
        for (int i = 28; i < 52; i++) {
            stockArray[++stockTop] = &allCards[i];
        }
        updateBlockedStatus();
        deleteSavedGame();
        currentState = PLAYING;
    }

    void updateBlockedStatus() {
        PyramidCard arr[28];
        pyramidBST.toArray(arr);
        for (int i = 0; i < 28; i++) {
            PyramidCard& pc = arr[i];
            if (pc.row == 6 || !pc.card->inPlay) {
                pc.blocked = false;
                continue;
            }
            PyramidCard leftChild;
            leftChild.row = pc.row + 1;
            leftChild.col = pc.col;
            BSTNode<PyramidCard>* leftNode = pyramidBST.search(leftChild);
            PyramidCard rightChild;
            rightChild.row = pc.row + 1;
            rightChild.col = pc.col + 1;
            BSTNode<PyramidCard>* rightNode = pyramidBST.search(rightChild);
            bool leftBlocked = (leftNode && leftNode->data.card->inPlay);
            bool rightBlocked = (rightNode && rightNode->data.card->inPlay);
            pc.blocked = leftBlocked || rightBlocked;
        }
    }

    bool isCardFree(PyramidCard* pc) {
        if (!pc || !pc->card || !pc->card->inPlay) return false;
        return !pc->blocked;
    }

    bool isValidMove(Card* c1, Card* c2) {
        if (!c1 || !c2) return false;
        if (!c1->inPlay || !c2->inPlay) return false;
        return (c1->value + c2->value) == 13;
    }

    bool isKing(Card* c) {
        if (!c) return false;
        return c->value == 13;
    }

    void drawCardFromStock() {
        moves++;
        if (stockTop < 0) {
            while (wasteTop >= 0) {
                Card* card = wasteArray[wasteTop--];
                if (card->inPlay) {
                    card->faceUp = false;
                    stockArray[++stockTop] = card;
                }
            }
            currentWasteCard = nullptr;
        }
        if (stockTop >= 0) {
            Card* card = stockArray[stockTop--];
            card->faceUp = true;
            wasteArray[++wasteTop] = card;
            currentWasteCard = card;
        }
    }

    void removeCards() {
        if (selectedCard1 && isKing(selectedCard1)) {
            selectedCard1->inPlay = false;

            if (currentWasteCard == selectedCard1) {
                currentWasteCard = nullptr;
                for (int i = wasteTop - 1; i >= 0; i--) {
                    if (wasteArray[i]->inPlay) {
                        currentWasteCard = wasteArray[i];
                        break;
                    }
                }
            }
            score += 10;
            moves++;
            if (currentWasteCard && !currentWasteCard->inPlay) currentWasteCard = nullptr;
            selectedCard1 = nullptr;
            selectedPyramid1 = nullptr;
            updateBlockedStatus();
            checkWinCondition();
            return;
        }
        if (selectedCard1 && selectedCard2 && isValidMove(selectedCard1, selectedCard2)) {
            selectedCard1->inPlay = false;
            selectedCard2->inPlay = false;

            if (currentWasteCard == selectedCard1 || currentWasteCard == selectedCard2) {
                currentWasteCard = nullptr;
                for (int i = wasteTop; i >= 0; i--) {
                    if (wasteArray[i]->inPlay) {
                        currentWasteCard = wasteArray[i];
                        break;
                    }
                }
            }
            score += 20;
            moves++;
            if (currentWasteCard && !currentWasteCard->inPlay) currentWasteCard = nullptr;
            selectedCard1 = selectedCard2 = nullptr;
            selectedPyramid1 = selectedPyramid2 = nullptr;
            updateBlockedStatus();
            checkWinCondition();
        }
        else if (selectedCard1 && selectedCard2) {
            moves++;
            selectedCard1 = selectedCard2 = nullptr;
            selectedPyramid1 = selectedPyramid2 = nullptr;
        }
    }

    void selectCard(Card* card, PyramidCard* pc) {
        if (!card || !card->inPlay) return;
        if (pc && !isCardFree(pc)) return;
        if (isKing(card)) {
            selectedCard1 = card;
            selectedPyramid1 = pc;
            removeCards();
            return;
        }
        if (!selectedCard1) {
            selectedCard1 = card;
            selectedPyramid1 = pc;
        }
        else if (selectedCard1 == card) {
            selectedCard1 = nullptr;
            selectedPyramid1 = nullptr;
        }
        else {
            selectedCard2 = card;
            selectedPyramid2 = pc;
            removeCards();
        }
    }

    void checkWinCondition() {
        bool allRemoved = true;
        for (int i = 0; i < 28; i++) {
            if (allPyramidCards[i].card && allPyramidCards[i].card->inPlay) {
                gameWon = false;
                break;
            }
        }
        if (allRemoved) {
            gameWon = true;
            saveCurrentGameScore();
            deleteSavedGame();
            isNewGame = true;
            currentGameScoreIndex = -1;
        }
    }


    void checkLoseCondition() {
        PyramidCard arr[28];
        pyramidBST.toArray(arr);
        Card* freeCards[82];
        int freeCount = 0;
        for (int i = 0; i < 28; i++) {
            if (isCardFree(&allPyramidCards[i])) {
                freeCards[freeCount++] = allPyramidCards[i].card;
            }
        }
        PyramidCard& pc = arr[i];
        if (pc.card->inPlay && !pc.blocked) {
            freeCards[freeCount++] = pc.card;
        }

        if (currentWasteCard && currentWasteCard->inPlay)
            freeCards[freeCount++] = currentWasteCard;

        for (int i = 0; i <= stockTop; i++) {
            if (stockArray[i]->inPlay) {
                freeCards[freeCount++] = stockArray[i];
            }
        }
        for (int i = 0; i <= wasteTop; i++) {
            if (wasteArray[i] != currentWasteCard && wasteArray[i]->inPlay) {
                freeCards[freeCount++] = wasteArray[i];
            }
        }
        if (freeCount == 0) {
            gameLost = true;
            saveCurrentGameScore();
            deleteSavedGame();
            isNewGame = true;
            currentGameScoreIndex = -1;
            return;
        }
        saveCurrentGameScore();
        deleteSavedGame();
        isNewGame = true;
        currentGameScoreIndex = -1;


        for (int i = 0; i < freeCount; i++) {
            if (isKing(freeCards[i])) return;
        }
        for (int i = 0; i < freeCount; i++) {
            for (int j = i + 1; j < freeCount; j++) {
                if (isValidMove(freeCards[i], freeCards[j])) return;
            }
        }
        if (stockTop >= 0) return;
        gameLost = true;
        saveCurrentGameScore();
        deleteSavedGame();
        isNewGame = true;
        currentGameScoreIndex = -1;



        Rectangle getPyramidCardRect(int row, int col) {
            int startX = (GetScreenWidth() / 2) - (row * (CARD_WIDTH + CARD_SPACING) / 2);
            int x = startX + col * (CARD_WIDTH + CARD_SPACING);
            int y = 100 + row * (CARD_HEIGHT / 2 + CARD_SPACING);
            return { (float)x, (float)y, (float)CARD_WIDTH, (float)CARD_HEIGHT };
        }
    }
    void drawCard(Card* card, Rectangle rect, bool selected) {
        if (!card) return;
        if (!card->faceUp) {
            DrawRectangleRec(rect, DARKBLUE);
            DrawRectangleLinesEx(rect, 2, BLACK);
            DrawText("?", rect.x + rect.width / 2 - 10, rect.y + rect.height / 2 - 10, 30, WHITE);
            return;
        }
        Texture2D tex = cardTextures[card->suit][card->value - 1];
        if (tex.id != 0) {
            DrawTexturePro(tex, { 0, 0, (float)tex.width, (float)tex.height },
                rect, { 0, 0 }, 0, WHITE);
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

    void drawBackground() {
        if (background.id != 0) {
            DrawTexturePro(background,
                { 0, 0, (float)background.width, (float)background.height },
                { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
                { 0, 0 }, 0, WHITE);
        }
        else {
            ClearBackground(DARKGREEN);
        }
    }

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
        DrawText(" Ace(1) + Queen(12), 2 + Jack(11), 3 + 10,", sw / 2 - 350, y, 20, WHITE);
        y += spacing - 5;
        DrawText(" 4 + 9, 5 + 8, 6 + 7", sw / 2 - 350, y, 20, WHITE);
        y += spacing + 5;
        DrawText("3. Kings (13) can be removed individually.", sw / 2 - 350, y, 20, WHITE);
        y += spacing + 5;
        DrawText("4. Click STOCK to draw cards.", sw / 2 - 350, y, 20, WHITE);
        Rectangle backBtn = { (float)(sw / 2 - 100), (float)(sh - 120), 200, 50 };
        DrawRectangleRec(backBtn, DARKGRAY);
        DrawRectangleLinesEx(backBtn, 2, WHITE);
        DrawText("BACK TO MENU", sw / 2 - 85, sh - 105, 20, WHITE);
        EndDrawing();
    }

    void render() {
        if (currentState == MAIN_MENU) {
            renderMainMenu();
            return;
        }
        if (currentState == INSTRUCTIONS) {
            renderInstructions();
            return;
        }
        BeginDrawing();
        drawBackground();
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        DrawText(TextFormat("Score: %d", score), 20, 20, 25, GOLD);
        DrawText(TextFormat("Moves: %d", moves), sw - 150, 20, 25, YELLOW);
        PyramidCard arr[28];
        pyramidBST.toArray(arr);
        for (int i = 0; i < 28; i++) {
            PyramidCard& pc = arr[i];
            if (pc.card && pc.card->inPlay) {
                Rectangle rect = getPyramidCardRect(pc.row, pc.col);
                bool selected = (&pc == selectedPyramid1 || &pc == selectedPyramid2);
                drawCard(pc.card, rect, selected);
                if (pc.blocked) {
                    DrawRectangle(rect.x, rect.y, rect.width, 5, RED);
                }
            }
        }
        int uiStartY = 150 + 7 * (CARD_HEIGHT / 2 + CARD_SPACING);
        DrawText("WASTE", 50, uiStartY - 30, 20, WHITE);
        if (currentWasteCard && currentWasteCard->inPlay) {
            Rectangle wasteRect = { 50, (float)uiStartY, CARD_WIDTH, CARD_HEIGHT };
            bool selected = (currentWasteCard == selectedCard1 || currentWasteCard == selectedCard2);
            drawCard(currentWasteCard, wasteRect, selected);
        }
        else {
            Rectangle emptyRect = { 50, (float)uiStartY, CARD_WIDTH, CARD_HEIGHT };
            DrawRectangleLinesEx(emptyRect, 2, GRAY);
        }
        stockRect = { 180.0f, (float)uiStartY, (float)CARD_WIDTH, (float)CARD_HEIGHT };
        DrawText("STOCK", 180, uiStartY - 30, 20, WHITE);
        DrawText(TextFormat("(%d)", stockTop + 1), 190, uiStartY + CARD_HEIGHT + 5, 18, LIGHTGRAY);
        if (stockTexture.id != 0 && stockTop >= 0) {
            DrawTexturePro(stockTexture,
                { 0, 0, (float)stockTexture.width, (float)stockTexture.height },
                stockRect, { 0, 0 }, 0, WHITE);
        }
        else if (stockTop >= 0) {
            DrawRectangleRec(stockRect, BLUE);
            DrawRectangleLinesEx(stockRect, 2, WHITE);
            DrawText("STOCK", stockRect.x + 15, stockRect.y + 55, 18, WHITE);
        }
        else {
            DrawRectangleLinesEx(stockRect, 2, GRAY);
        }
        int totalSeconds = (int)gameTime;
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        DrawText(TextFormat("Time: %02d:%02d:%02d", hours, minutes, seconds),
            sw / 2 - 80, sh - 30, 25, WHITE);
        Rectangle restartBtn = { (float)(sw - 150), (float)(sh - 60), 120, 50 };
        DrawRectangleRec(restartBtn, MAROON);
        DrawRectangleLinesEx(restartBtn, 2, WHITE);
        DrawText("RESTART", sw - 140, sh - 45, 20, WHITE);
        if (gameWon) {
            DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });
            DrawText("YOU WIN!", sw / 2 - 100, sh / 2 - 50, 40, GOLD);
            DrawText(TextFormat("Final Score: %d", score), sw / 2 - 100, sh / 2 + 10, 30, WHITE);
            DrawText("Click anywhere for menu", sw / 2 - 150, sh / 2 + 60, 20, LIGHTGRAY);
        }
        else if (gameLost) {
            DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });
            DrawText("NO MOVES LEFT!", sw / 2 - 150, sh / 2 - 50, 40, RED);
            DrawText(TextFormat("Final Score: %d", score), sw / 2 - 100, sh / 2 + 10, 30, WHITE);
            DrawText("Click anywhere for menu", sw / 2 - 150, sh / 2 + 60, 20, LIGHTGRAY);
        }
        if (isPaused) {
            DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });
            DrawText("PAUSED", sw / 2 - 80, sh / 2, 40, YELLOW);
            DrawText("Press P to Resume", sw / 2 - 120, sh / 2 + 50, 25, WHITE);
        }
        EndDrawing();
    }

    void handleMouseClick(int mouseX, int mouseY) {
        if (gameWon || gameLost) {
            currentState = MAIN_MENU;
            return;
        }
        PyramidCard arr[28];
        pyramidBST.toArray(arr);
        for (int i = 0; i < 28; i++) {
            PyramidCard& pc = arr[i];
            if (pc.card && pc.card->inPlay) {
                Rectangle cardRect = getPyramidCardRect(pc.row, pc.col);
                if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, cardRect)) {
                    selectCard(pc.card, &pc);
                    return;
                }
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
            return;
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

    void loadHighScores() {
        ifstream file(SCORE_FILE);
        highScoreCount = 0;
        if (!file.is_open())
            return;
        while (highScoreCount < 5 && file >> highScores[highScoreCount]) {
            highScoreCount++;
        }
        file.close();

        for (int i = 0; i < highScoreCount - 1; i++) {

            for (int j = i + 1; j < highScoreCount; j++) {

                if (highScores[j] > highScores[i]) {
                    int temp = highScores[i];
                    highScores[i] = highScores[j];
                    highScores[j] = temp;
                }
            }
        }
    }

    void saveHighScores() {
        ofstream file(SCORE_FILE, ios::trunc);
        if (!file.is_open()) {
            cout << "Error: Could not save high scores!" << endl;
            return;
        }

        for (int i = 0; i < highScoreCount; i++) {
            file << highScores[i] << endl;
        }
        file.close();
        cout << "High scores saved successfully." << endl;
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
            }
            isNewGame = false;
        }
        else if (currentGameScoreIndex >= 0 && currentGameScoreIndex < 5) {

            highScores[currentGameScoreIndex] = score;
        }

        for (int i = 0; i < highScoreCount - 1; i++) {

            for (int j = i + 1; j < highScoreCount; j++) {

                if (highScores[j] > highScores[i]) {
                    int temp = highScores[i];
                    highScores[i] = highScores[j];
                    highScores[j] = temp;
                }
            }
        }

        saveHighScores();
    }


    void update(float deltaTime) {
        if (IsKeyPressed(KEY_BACKSPACE)) {

            if (currentState == PLAYING && !gameWon && !gameLost) {
                saveGame();
            }
            currentState = MAIN_MENU;
            isPaused = false;
            checkSavedGame();
            return;
        }

        if (IsKeyPressed(KEY_P) && !gameWon && !gameLost) {
            isPaused = !isPaused;
        }

        if (IsKeyPressed(KEY_S) && !gameWon && !gameLost && !isPaused) {

            saveGame();
        }

        if (isPaused) return;
        gameTime += deltaTime;
        checkLoseCondition();
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
    InitWindow(screenWidth, screenHeight, "Pyramid Solitaire");
    SetTargetFPS(60);
    PyramidSolitaire game;
    while (!WindowShouldClose()) {
        game.update(GetFrameTime());
        game.render();
    }
    CloseWindow();
    return 0;
}
