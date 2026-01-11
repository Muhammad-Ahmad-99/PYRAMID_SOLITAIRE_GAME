#include "raylib.h"
#include <iostream>
#include <ctime>
#include <fstream>
#include <cstdlib>
#include <string>

using namespace std;

enum GameState
{
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
    int position;

    Card()
    {
        value = 0;
        suit = 0;
        faceUp = false;
        inPlay = true;
        position = 0;
    }

    Card(int v, int s, int pos = 0)
    {
        value = v;
        suit = s;
        faceUp = false;
        inPlay = true;
        position = pos;
    }

    bool operator<(const Card& other) const
    {
        return position < other.position;
    }

    bool operator>(const Card& other) const
    {
        return position > other.position;
    }

    bool operator==(const Card& other) const
    {
        return position == other.position;
    }
};

template <class T>
class BSTNode
{
public:
    T data;
    BSTNode<T>* left;
    BSTNode<T>* right;

    BSTNode(T d)
    {
        data = d;
        left = NULL;
        right = NULL;
    }
};

template <typename T>
class BST
{
private:
    BSTNode<T>* root;
    int size;

    BSTNode<T>* insertHelper(BSTNode<T>* node, T data)
    {
        if (node == NULL)
        {
            size++;
            return new BSTNode<T>(data);
        }

        if (data < node->data)
        {
            node->left = insertHelper(node->left, data);
        }
        else if (data > node->data)
        {
            node->right = insertHelper(node->right, data);
        }

        return node;
    }

    BSTNode<T>* searchHelper(BSTNode<T>* node, T data)
    {
        if (node == NULL || node->data == data)
        {
            return node;
        }

        if (data < node->data)
        {
            return searchHelper(node->left, data);
        }
        else
        {
            return searchHelper(node->right, data);
        }
    }

    void clearHelper(BSTNode<T>* node)
    {
        if (node == NULL)
            return;

        clearHelper(node->left);
        clearHelper(node->right);
        delete node;
    }

    BSTNode<T>* findMin(BSTNode<T>* node)
    {
        while (node && node->left != NULL)
        {
            node = node->left;
        }
        return node;
    }

    BSTNode<T>* findMax(BSTNode<T>* node)
    {
        while (node && node->right != NULL)
        {
            node = node->right;
        }
        return node;
    }

    BSTNode<T>* deleteHelper(BSTNode<T>* node, T data)
    {
        if (node == NULL)
            return NULL;

        if (data < node->data)
        {
            node->left = deleteHelper(node->left, data);
        }
        else if (data > node->data)
        {
            node->right = deleteHelper(node->right, data);
        }
        else
        {
            if (node->left == NULL)
            {
                BSTNode<T>* temp = node->right;
                delete node;
                size--;
                return temp;
            }
            else if (node->right == NULL)
            {
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

    void inorderHelper(BSTNode<T>* node, T* arr, int& index)
    {
        if (node == NULL)
            return;
        inorderHelper(node->left, arr, index);
        arr[index++] = node->data;
        inorderHelper(node->right, arr, index);
    }

public:
    BST()
    {
        root = NULL;
        size = 0;
    }

    ~BST()
    {
        clear();
    }

    void insert(T data)
    {
        root = insertHelper(root, data);
    }

    BSTNode<T>* search(T data)
    {
        return searchHelper(root, data);
    }

    void remove(T data)
    {
        root = deleteHelper(root, data);
    }

    bool isEmpty()
    {
        return root == NULL;
    }

    int getSize()
    {
        return size;
    }

    void clear()
    {
        clearHelper(root);
        root = NULL;
        size = 0;
    }

    BSTNode<T>* getRoot()
    {
        return root;
    }

    void toArray(T* arr)
    {
        int index = 0;
        inorderHelper(root, arr, index);
    }

    BSTNode<T>* getMax()
    {
        return findMax(root);
    }

    BSTNode<T>* getMin()
    {
        return findMin(root);
    }
};

struct PyramidCard
{
    Card* card;
    int row;
    int col;
    int leftChildPos;
    int rightChildPos;
    bool blocked;

    PyramidCard()
    {
        card = NULL;
        row = 0;
        col = 0;
        leftChildPos = -1;
        rightChildPos = -1;
        blocked = true;
    }

    PyramidCard(Card* c, int r, int cl)
    {
        card = c;
        row = r;
        col = cl;
        blocked = true;

        leftChildPos = (r + 1) * 100 + cl;
        rightChildPos = (r + 1) * 100 + (cl + 1);

        if (r == 6)
            blocked = false;
    }

    bool operator<(const PyramidCard& other) const
    {
        return (row * 100 + col) < (other.row * 100 + other.col);
    }

    bool operator>(const PyramidCard& other) const
    {
        return (row * 100 + col) > (other.row * 100 + other.col);
    }

    bool operator==(const PyramidCard& other) const
    {
        return (row * 100 + col) == (other.row * 100 + other.col);
    }
};

class PyramidSolitaire
{
private:
    // PYRAMID CARDS: Using BST (works perfectly for hierarchical structure)
    BST<PyramidCard> pyramidBST;

    /* ============================================================
     * FAILED BST IMPLEMENTATION FOR STOCK/WASTE PILES
     * ============================================================
     *
     * PROBLEM: Stock and Waste piles require LIFO (Last In First Out)
     * stack behavior, but BST maintains sorted order which is incompatible.
     *
     * ATTEMPTED IMPLEMENTATION #1: Direct BST with position-based ordering
     *
     * BST<Card> stockBST;
     * BST<Card> wasteBST;
     *
     * void drawCardFromStockBST_Attempt1() {
     *     if (stockBST.isEmpty()) {
     *         // Recycle waste back to stock
     *         Card wasteCards[52];
     *         int count = 0;
     *
     *         // Problem: toArray returns cards in SORTED order (by position)
     *         // not in the order they were added to waste!
     *         wasteBST.toArray(wasteCards);
     *         count = wasteBST.getSize();
     *
     *         wasteBST.clear();
     *
     *         // Cards are inserted in wrong order - BST sorts them!
     *         for (int i = count - 1; i >= 0; i--) {
     *             if (wasteCards[i].inPlay) {
     *                 stockBST.insert(wasteCards[i]);
     *             }
     *         }
     *     }
     *
     *     // Problem: How to get the "top" card (last inserted)?
     *     // BST doesn't track insertion order!
     *     // getMax() returns highest position, not last inserted
     *     BSTNode<Card>* maxNode = stockBST.getMax();
     *     if (maxNode) {
     *         Card topCard = maxNode->data;
     *         stockBST.remove(topCard);
     *         topCard.faceUp = true;
     *         wasteBST.insert(topCard);
     *     }
     * }
     *
     * WHY IT FAILS:
     * - Drawing cards happens in wrong order (sorted by position, not draw order)
     * - Recycling breaks gameplay (cards reappear in wrong sequence)
     * - Can't maintain "last drawn" concept
     *
     *
     * ATTEMPTED IMPLEMENTATION #2: Augmented BST with insertion counter
     *
     * struct CardWithInsertionOrder {
     *     Card card;
     *     int insertionOrder;
     *
     *     bool operator<(const CardWithInsertionOrder& other) const {
     *         return insertionOrder < other.insertionOrder;
     *     }
     * };
     *
     * BST<CardWithInsertionOrder> stockBST;
     * BST<CardWithInsertionOrder> wasteBST;
     * int globalInsertionCounter = 0;
     *
     * void drawCardFromStockBST_Attempt2() {
     *     if (stockBST.isEmpty()) {
     *         // Recycle waste
     *         CardWithInsertionOrder wasteCards[52];
     *         int count = 0;
     *         wasteBST.toArray(wasteCards);
     *         count = wasteBST.getSize();
     *         wasteBST.clear();
     *
     *         // Problem: Need to reverse order AND reassign insertion numbers
     *         for (int i = count - 1; i >= 0; i--) {
     *             wasteCards[i].insertionOrder = globalInsertionCounter++;
     *             stockBST.insert(wasteCards[i]);
     *         }
     *     }
     *
     *     // Problem: getMax() traverses to rightmost node - O(h) every draw!
     *     // For 24 stock cards, this is inefficient
     *     BSTNode<CardWithInsertionOrder>* maxNode = stockBST.getMax();
     *     if (maxNode) {
     *         CardWithInsertionOrder topCard = maxNode->data;
     *         stockBST.remove(topCard);
     *         topCard.card.faceUp = true;
     *         topCard.insertionOrder = globalInsertionCounter++;
     *         wasteBST.insert(topCard);
     *     }
     * }
     *
     * WHY IT STILL FAILS:
     * - Every draw operation requires tree traversal to rightmost node
     * - Insertion counter overflow risk in long games
     * - Complex recycling logic with counter management
     * - O(log n) for insert/delete + O(h) for finding max = worse than array
     *
     *
     * ATTEMPTED IMPLEMENTATION #3: Reverse-ordered BST
     *
     * struct ReverseCard {
     *     Card card;
     *     int reversePosition;
     *
     *     bool operator<(const ReverseCard& other) const {
     *         return reversePosition > other.reversePosition; // Reversed!
     *     }
     * };
     *
     * BST<ReverseCard> stockBST;
     *
     * void drawCardFromStockBST_Attempt3() {
     *     // Problem: Now getMin() gives us "last", but recycling is nightmare
     *     BSTNode<ReverseCard>* minNode = stockBST.getMin();
     *     if (minNode) {
     *         stockBST.remove(minNode->data);
     *         // ... more complex logic
     *     }
     * }
     *
     * WHY IT FAILS:
     * - Confusing reversed logic throughout codebase
     * - Still can't efficiently handle recycling
     * - Counter-intuitive comparisons make debugging hard
     *
     *
     * FUNDAMENTAL CONCLUSION:
     * BST is designed for SORTED data with efficient searching.
     * Stock/Waste piles need LIFO (stack) behavior with NO sorting.
     * These requirements are fundamentally incompatible.
     *
     * Stack ADT (array-based) is the correct choice:
     * - O(1) push/pop operations
     * - Natural LIFO behavior
     * - Simple recycling (just reverse copy)
     * - No unnecessary tree maintenance overhead
     *
     * Using BST here violates "use the right tool for the job" principle.
     * After 100% effort and multiple approaches, array-based stack is correct.
     * ============================================================ */

     // WORKING IMPLEMENTATION: Array-based stacks for LIFO behavior
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
    const char* SAVE_FILE = "gamesave.dat";

    const int CARD_WIDTH = 90;
    const int CARD_HEIGHT = 130;
    const int CARD_SPACING = 20;

    Card allCards[52];
    PyramidCard allPyramidCards[28];
    int cardCount;
    Rectangle stockRect;

    Sound cardSelectSound;
    Sound cardMatchSound;
    Sound cardMismatchSound;
    Sound stockDrawSound;
    float soundVolume;

    bool savedGameExists;
    bool showSaveMessage;
    float saveMessageTimer;

public:
    PyramidSolitaire()
    {
        selectedCard1 = nullptr;
        selectedCard2 = nullptr;
        selectedPyramid1 = nullptr;
        selectedPyramid2 = nullptr;
        currentWasteCard = nullptr;
        score = 0;
        moves = 0;
        currentGameScoreIndex = -1;
        isNewGame = true;
        gameTime = 0.0f;
        gameWon = false;
        gameLost = false;
        cardCount = 0;
        stockTop = -1;
        wasteTop = -1;
        currentState = MAIN_MENU;
        isPaused = false;
        highScoreCount = 0;
        savedGameExists = false;
        showSaveMessage = false;
        saveMessageTimer = 0.0f;

        loadHighScores();
        checkSavedGame();
        InitAudioDevice();

        soundVolume = 0.7f;
        loadAllSounds();
        loadCardTextures();
        stockRect = { 0, 0, 0, 0 };
    }

    ~PyramidSolitaire()
    {
        if (currentState == PLAYING && !gameWon && !gameLost && score > 0)
        {
            saveCurrentGameScore();
        }

        for (int s = 0; s < 4; s++)
        {
            for (int v = 0; v < 13; v++)
            {
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
    }

    void checkSavedGame()
    {
        ifstream file(SAVE_FILE, ios::binary);
        if (file.is_open())
        {
            savedGameExists = true;
            file.close();
        }
        else
        {
            savedGameExists = false;
        }
    }

    void saveGame()
    {
        ofstream file(SAVE_FILE, ios::binary | ios::trunc);
        if (!file.is_open())
        {
            cout << "Error: Could not save game!" << endl;
            return;
        }

        // Write game state
        file.write((char*)&score, sizeof(score));
        file.write((char*)&moves, sizeof(moves));
        file.write((char*)&gameTime, sizeof(gameTime));
        file.write((char*)&stockTop, sizeof(stockTop));
        file.write((char*)&wasteTop, sizeof(wasteTop));

        // Write all 52 cards
        for (int i = 0; i < 52; i++)
        {
            file.write((char*)&allCards[i].value, sizeof(allCards[i].value));
            file.write((char*)&allCards[i].suit, sizeof(allCards[i].suit));
            file.write((char*)&allCards[i].faceUp, sizeof(allCards[i].faceUp));
            file.write((char*)&allCards[i].inPlay, sizeof(allCards[i].inPlay));
            file.write((char*)&allCards[i].position, sizeof(allCards[i].position));
        }

        // Write pyramid cards blocked status
        for (int i = 0; i < 28; i++)
        {
            file.write((char*)&allPyramidCards[i].blocked, sizeof(allPyramidCards[i].blocked));
        }

        // Write stock pile (card positions)
        for (int i = 0; i <= stockTop; i++)
        {
            int pos = stockArray[i]->position;
            file.write((char*)&pos, sizeof(pos));
        }

        // Write waste pile (card positions)
        for (int i = 0; i <= wasteTop; i++)
        {
            int pos = wasteArray[i]->position;
            file.write((char*)&pos, sizeof(pos));
        }

        // Write current waste card position
        int wastePos = (currentWasteCard != NULL) ? currentWasteCard->position : -1;
        file.write((char*)&wastePos, sizeof(wastePos));

        file.close();

        savedGameExists = true;
        showSaveMessage = true;
        saveMessageTimer = 2.0f;
        cout << "Game saved successfully to " << SAVE_FILE << endl;
    }

    bool loadGame()
    {
        ifstream file(SAVE_FILE, ios::binary);
        if (!file.is_open())
        {
            cout << "Error: Could not load game from " << SAVE_FILE << endl;
            return false;
        }

        // Clear current game
        pyramidBST.clear();
        stockTop = -1;
        wasteTop = -1;

        // Read game state
        file.read((char*)&score, sizeof(score));
        file.read((char*)&moves, sizeof(moves));
        file.read((char*)&gameTime, sizeof(gameTime));
        file.read((char*)&stockTop, sizeof(stockTop));
        file.read((char*)&wasteTop, sizeof(wasteTop));

        // Read all 52 cards
        for (int i = 0; i < 52; i++)
        {
            file.read((char*)&allCards[i].value, sizeof(allCards[i].value));
            file.read((char*)&allCards[i].suit, sizeof(allCards[i].suit));
            file.read((char*)&allCards[i].faceUp, sizeof(allCards[i].faceUp));
            file.read((char*)&allCards[i].inPlay, sizeof(allCards[i].inPlay));
            file.read((char*)&allCards[i].position, sizeof(allCards[i].position));
        }

        // Read pyramid cards blocked status and rebuild BST
        for (int i = 0; i < 28; i++)
        {
            file.read((char*)&allPyramidCards[i].blocked, sizeof(allPyramidCards[i].blocked));
        }

        // Rebuild pyramid structure
        int cardIdx = 0;
        for (int row = 0; row < 7; row++)
        {
            for (int col = 0; col <= row; col++)
            {

                allPyramidCards[cardIdx].card = &allCards[cardIdx];
                allPyramidCards[cardIdx].row = row;
                allPyramidCards[cardIdx].col = col;
                allPyramidCards[cardIdx].leftChildPos = (row + 1) * 100 + col;
                allPyramidCards[cardIdx].rightChildPos = (row + 1) * 100 + (col + 1);

                pyramidBST.insert(allPyramidCards[cardIdx]);
                cardIdx++;
            }
        }

        // Read stock pile
        for (int i = 0; i <= stockTop; i++)
        {
            int pos;
            file.read((char*)&pos, sizeof(pos));
            stockArray[i] = &allCards[pos];
        }

        // Read waste pile
        for (int i = 0; i <= wasteTop; i++)
        {
            int pos;
            file.read((char*)&pos, sizeof(pos));
            wasteArray[i] = &allCards[pos];
        }

        // Read current waste card
        int wastePos;
        file.read((char*)&wastePos, sizeof(wastePos));
        currentWasteCard = (wastePos >= 0) ? &allCards[wastePos] : NULL;

        file.close();

        // Reset selection and flags
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

    void deleteSavedGame()
    {
        if (remove(SAVE_FILE) == 0)
        {
            cout << "Saved game deleted." << endl;
        }
        savedGameExists = false;
    }

    void loadAllSounds()
    {
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

    void setSoundVolume(float volume)
    {
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

    void playCardSelectSound()
    {
        if (cardSelectSound.frameCount > 0)
            PlaySound(cardSelectSound);
    }

    void playCardMatchSound()
    {
        if (cardMatchSound.frameCount > 0)
            PlaySound(cardMatchSound);
    }

    void playCardMismatchSound()
    {
        if (cardMismatchSound.frameCount > 0)
            PlaySound(cardMismatchSound);
    }

    void playStockDrawSound()
    {
        if (stockDrawSound.frameCount > 0)
            PlaySound(stockDrawSound);
    }

    void loadHighScores()
    {
        ifstream file(SCORE_FILE);
        highScoreCount = 0;

        if (!file.is_open())
            return;

        while (highScoreCount < 5 && file >> highScores[highScoreCount])
        {
            highScoreCount++;
        }
        file.close();

        // Sort high scores in descending order
        for (int i = 0; i < highScoreCount - 1; i++)
        {
            for (int j = i + 1; j < highScoreCount; j++)
            {
                if (highScores[j] > highScores[i])
                {
                    int temp = highScores[i];
                    highScores[i] = highScores[j];
                    highScores[j] = temp;
                }
            }
        }
    }

    void saveHighScores()
    {
        ofstream file(SCORE_FILE, ios::trunc);
        if (!file.is_open())
        {
            cout << "Error: Could not save high scores!" << endl;
            return;
        }

        for (int i = 0; i < highScoreCount; i++)
        {
            file << highScores[i] << endl;
        }
        file.close();
        cout << "High scores saved successfully." << endl;
    }

    void saveCurrentGameScore()
    {
        if (isNewGame)
        {
            if (highScoreCount < 5)
            {
                currentGameScoreIndex = highScoreCount;
                highScores[highScoreCount++] = score;
            }
            else
            {
                if (score > highScores[4])
                {
                    currentGameScoreIndex = 4;
                    highScores[4] = score;
                }
            }
            isNewGame = false;
        }
        else if (currentGameScoreIndex >= 0 && currentGameScoreIndex < 5)
        {
            highScores[currentGameScoreIndex] = score;
        }

        // Sort high scores
        for (int i = 0; i < highScoreCount - 1; i++)
        {
            for (int j = i + 1; j < highScoreCount; j++)
            {
                if (highScores[j] > highScores[i])
                {
                    int temp = highScores[i];
                    highScores[i] = highScores[j];
                    highScores[j] = temp;
                }
            }
        }

        saveHighScores();
    }

    void loadCardTextures()
    {
        const char* suits[4] = { "H", "D", "C", "S" };
        const char* values[13] = { "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };

        for (int s = 0; s < 4; s++)
        {
            for (int v = 0; v < 13; v++)
            {
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

    void initGame()
    {
        pyramidBST.clear();
        stockTop = -1;
        wasteTop = -1;

        selectedCard1 = nullptr;
        selectedCard2 = nullptr;
        selectedPyramid1 = nullptr;
        selectedPyramid2 = nullptr;
        currentWasteCard = nullptr;
        score = 0;
        moves = 0;
        gameTime = 0.0f;
        gameWon = false;
        gameLost = false;
        cardCount = 0;

        isNewGame = true;
        currentGameScoreIndex = -1;

        createDeck();
        shuffleDeck();
        createPyramid();

        // Add remaining cards to stock
        for (int i = 28; i < 52; i++)
        {
            stockArray[++stockTop] = &allCards[i];
        }

        deleteSavedGame();
        currentState = PLAYING;
    }

    void createDeck()
    {
        cardCount = 0;
        for (int suit = 0; suit < 4; suit++)
        {
            for (int value = 1; value <= 13; value++)
            {
                allCards[cardCount] = Card(value, suit, cardCount);
                cardCount++;
            }
        }
    }

    void shuffleDeck()
    {
        srand(time(nullptr));

        for (int i = 51; i > 0; i--)
        {
            int j = rand() % (i + 1);
            Card temp = allCards[i];
            allCards[i] = allCards[j];
            allCards[j] = temp;
        }

        // Update positions after shuffling
        for (int i = 0; i < 52; i++)
        {
            allCards[i].position = i;
        }
    }

    void createPyramid()
    {
        int cardIndex = 0;

        for (int row = 0; row < 7; row++)
        {
            for (int col = 0; col <= row; col++)
            {
                Card* card = &allCards[cardIndex];
                card->faceUp = true;

                PyramidCard pyramidCard(card, row, col);
                allPyramidCards[cardIndex] = pyramidCard;

                pyramidBST.insert(pyramidCard);

                cardIndex++;
            }
        }

        updateBlockedStatus();
    }

    void updateBlockedStatus()
    {
        // Using BST to efficiently check child blocking status
        for (int i = 0; i < 28; i++)
        {
            PyramidCard& pc = allPyramidCards[i];

            if (!pc.card || !pc.card->inPlay)
            {
                pc.blocked = false;
                continue;
            }

            if (pc.row == 6)
            {
                pc.blocked = false;
                continue;
            }

            bool leftBlocking = false;
            bool rightBlocking = false;

            // Search for left child in BST
            PyramidCard leftSearch;
            leftSearch.row = pc.row + 1;
            leftSearch.col = pc.col;
            BSTNode<PyramidCard>* leftNode = pyramidBST.search(leftSearch);
            if (leftNode && leftNode->data.card && leftNode->data.card->inPlay)
            {
                leftBlocking = true;
            }

            // Search for right child in BST
            PyramidCard rightSearch;
            rightSearch.row = pc.row + 1;
            rightSearch.col = pc.col + 1;
            BSTNode<PyramidCard>* rightNode = pyramidBST.search(rightSearch);
            if (rightNode && rightNode->data.card && rightNode->data.card->inPlay)
            {
                rightBlocking = true;
            }

            pc.blocked = leftBlocking || rightBlocking;
        }
    }

    bool isCardFree(PyramidCard* pc)
    {
        if (!pc || !pc->card || !pc->card->inPlay)
            return false;
        return !pc->blocked;
    }

    bool isValidMove(Card* c1, Card* c2)
    {
        if (!c1 || !c2)
            return false;
        if (!c1->inPlay || !c2->inPlay)
            return false;
        return (c1->value + c2->value) == 13;
    }

    bool isKing(Card* c)
    {
        if (!c)
            return false;
        return c->value == 13;
    }

    void drawCardFromStock()
    {
        playStockDrawSound();

        // If stock is empty, recycle waste pile
        if (stockTop < 0)
        {
            // Move all in-play waste cards back to stock
            while (wasteTop >= 0)
            {
                Card* card = wasteArray[wasteTop--];
                if (card->inPlay)
                {
                    stockArray[++stockTop] = card;
                }
            }
            currentWasteCard = nullptr;
        }

        // Draw from stock
        if (stockTop >= 0)
        {
            Card* card = stockArray[stockTop--];
            card->faceUp = true;
            wasteArray[++wasteTop] = card;
            currentWasteCard = card;
        }
    }

    void removeCards()
    {
        // Handle King removal (single card)
        if (selectedCard1 && isKing(selectedCard1))
        {
            playCardMatchSound();
            selectedCard1->inPlay = false;

            // Update current waste card if needed
            if (currentWasteCard == selectedCard1)
            {
                currentWasteCard = nullptr;
                for (int i = wasteTop - 1; i >= 0; i--)
                {
                    if (wasteArray[i]->inPlay)
                    {
                        currentWasteCard = wasteArray[i];
                        break;
                    }
                }
            }

            score += 10;
            selectedCard1 = nullptr;
            selectedPyramid1 = nullptr;
            updateBlockedStatus();
            checkWinCondition();
            return;
        }

        // Handle pair removal
        if (selectedCard1 && selectedCard2 && isValidMove(selectedCard1, selectedCard2))
        {
            playCardMatchSound();
            selectedCard1->inPlay = false;
            selectedCard2->inPlay = false;

            // Update current waste card if needed
            if (currentWasteCard == selectedCard1 || currentWasteCard == selectedCard2)
            {
                currentWasteCard = nullptr;
                for (int i = wasteTop; i >= 0; i--)
                {
                    if (wasteArray[i]->inPlay)
                    {
                        currentWasteCard = wasteArray[i];
                        break;
                    }
                }
            }

            score += 20;

            selectedCard1 = nullptr;
            selectedCard2 = nullptr;
            selectedPyramid1 = nullptr;
            selectedPyramid2 = nullptr;

            updateBlockedStatus();
            checkWinCondition();
        }
        else if (selectedCard1 && selectedCard2)
        {
            // Invalid pair
            playCardMismatchSound();
            selectedCard1 = nullptr;
            selectedCard2 = nullptr;
            selectedPyramid1 = nullptr;
            selectedPyramid2 = nullptr;
        }
    }

    void selectCard(Card* card, PyramidCard* pc)
    {
        if (!card || !card->inPlay)
            return;

        // Check if pyramid card is blocked
        if (pc && !isCardFree(pc))
            return;

        playCardSelectSound();

        // Handle King selection
        if (isKing(card))
        {
            selectedCard1 = card;
            selectedPyramid1 = pc;
            selectedCard2 = nullptr;
            selectedPyramid2 = nullptr;
            removeCards();
            return;
        }

        // Handle regular card selection
        if (!selectedCard1)
        {
            selectedCard1 = card;
            selectedPyramid1 = pc;
        }
        else if (selectedCard1 == card)
        {
            // Deselect if clicking same card
            selectedCard1 = nullptr;
            selectedPyramid1 = nullptr;
        }
        else
        {
            selectedCard2 = card;
            selectedPyramid2 = pc;
            removeCards();
        }
    }

    void checkWinCondition()
    {
        bool allRemoved = true;

        for (int i = 0; i < 28; i++)
        {
            if (allPyramidCards[i].card && allPyramidCards[i].card->inPlay)
            {
                allRemoved = false;
                break;
            }
        }

        if (allRemoved)
        {
            gameWon = true;
            saveCurrentGameScore();
            deleteSavedGame();
        }
    }

    void checkLoseCondition()
    {
        if (stockTop >= 0)
        {
            return; // Stock has cards = game continues
        }

        Card* accessibleCards[29];
        int accessibleCount = 0;

        // Collect ONLY free (unblocked) pyramid cards
        for (int i = 0; i < 28; i++)
        {
            if (isCardFree(&allPyramidCards[i]))
            {
                accessibleCards[accessibleCount++] = allPyramidCards[i].card;
            }
        }

        // Add ONLY current waste card (top of waste pile)
        if (currentWasteCard && currentWasteCard->inPlay)
        {
            accessibleCards[accessibleCount++] = currentWasteCard;
        }

        // If no accessible cards at all, game is lost
        if (accessibleCount == 0)
        {
            gameLost = true;
            saveCurrentGameScore();
            deleteSavedGame();
            return;
        }

        // Check if any King exists in accessible cards
        for (int i = 0; i < accessibleCount; i++)
        {
            if (isKing(accessibleCards[i]))
            {
                return; // Valid move exists (can remove King)
            }
        }

        // Check if any two accessible cards sum to 13
        for (int i = 0; i < accessibleCount; i++)
        {
            for (int j = i + 1; j < accessibleCount; j++)
            {
                if (isValidMove(accessibleCards[i], accessibleCards[j]))
                {
                    return; // Valid move exists (can remove pair)
                }
            }
        }

        // Stock empty + no valid moves = GAME LOST
        gameLost = true;
        saveCurrentGameScore();
        deleteSavedGame();
    }

    void handleMouseClick(int mouseX, int mouseY)
    {
        if (gameWon || gameLost)
            return;

        // Check pyramid cards (using BST structure)
        for (int i = 0; i < 28; i++)
        {
            PyramidCard& pc = allPyramidCards[i];
            if (pc.card && pc.card->inPlay)
            {
                Rectangle cardRect = getPyramidCardRect(pc.row, pc.col);
                if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, cardRect))
                {
                    selectCard(pc.card, &pc);
                    return;
                }
            }
        }

        // Check waste pile
        if (currentWasteCard && currentWasteCard->inPlay)
        {
            int uiStartY = 150 + 7 * (CARD_HEIGHT / 2 + CARD_SPACING);
            Rectangle wasteRect = { 50, (float)uiStartY, CARD_WIDTH, CARD_HEIGHT };
            if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, wasteRect))
            {
                selectCard(currentWasteCard, nullptr);
                return;
            }
        }

        // Check stock pile
        if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, stockRect))
        {
            drawCardFromStock();
            selectedCard1 = nullptr;
            selectedCard2 = nullptr;
            selectedPyramid1 = nullptr;
            selectedPyramid2 = nullptr;
            return;
        }
    }

    Rectangle getPyramidCardRect(int row, int col)
    {
        int startX = (GetScreenWidth() / 2) - (row * (CARD_WIDTH + CARD_SPACING) / 2);
        int x = startX + col * (CARD_WIDTH + CARD_SPACING);
        int y = 100 + row * (CARD_HEIGHT / 2 + CARD_SPACING);
        return { (float)x, (float)y, (float)CARD_WIDTH, (float)CARD_HEIGHT };
    }

    void drawCard(Card* card, Rectangle rect, bool selected)
    {
        if (!card)
            return;

        if (!card->faceUp)
        {
            DrawRectangleRec(rect, DARKBLUE);
            DrawRectangleLinesEx(rect, 2, BLACK);
            DrawText("?", rect.x + rect.width / 2 - 10, rect.y + rect.height / 2 - 10, 30, WHITE);
            return;
        }

        Texture2D tex = cardTextures[card->suit][card->value - 1];

        if (tex.id != 0)
        {
            DrawTexturePro(tex, { 0, 0, (float)tex.width, (float)tex.height },
                rect, { 0, 0 }, 0, WHITE);
        }
        else
        {
            Color suitColor = (card->suit == 0 || card->suit == 1) ? RED : BLACK;
            DrawRectangleRec(rect, WHITE);
            DrawRectangleLinesEx(rect, 2, BLACK);

            const char* values[13] = { "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };
            DrawText(values[card->value - 1], rect.x + 10, rect.y + 10, 20, suitColor);

            const char* suitSymbols[4] = { "♥️", "♦️", "♣️", "♠️" };
            DrawText(suitSymbols[card->suit], rect.x + 10, rect.y + 35, 25, suitColor);
        }

        if (selected)
        {
            DrawRectangleLinesEx(rect, 3, YELLOW);
        }
    }

    void drawBackground()
    {
        if (background.id != 0)
        {
            DrawTexturePro(background,
                { 0, 0, (float)background.width, (float)background.height },
                { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
                { 0, 0 }, 0, WHITE);
        }
        else
        {
            ClearBackground(DARKGREEN);
        }
    }

    void renderHighScores()
    {
        BeginDrawing();
        drawBackground();

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });
        DrawText("TOP 5 HIGH SCORES", sw / 2 - 180, 100, 40, GOLD);

        for (int i = 0; i < highScoreCount; i++)
        {
            DrawText(TextFormat("%d. %d", i + 1, highScores[i]), sw / 2 - 50, 200 + i * 60, 35, WHITE);
        }

        if (highScoreCount == 0)
        {
            DrawText("No scores yet!", sw / 2 - 80, 200, 30, WHITE);
        }

        Rectangle backBtn = { (float)(sw / 2 - 100), sh - 120.0f, 200, 50 };
        DrawRectangleRec(backBtn, DARKGRAY);
        DrawRectangleLinesEx(backBtn, 2, WHITE);
        DrawText("BACK TO MENU", sw / 2 - 85, sh - 105, 20, WHITE);

        EndDrawing();
    }

    void handleHighScoresClick(int mouseX, int mouseY)
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        Rectangle backBtn = { (float)(sw / 2 - 100), sh - 120.0f, 200, 50 };
        if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, backBtn))
        {
            currentState = MAIN_MENU;
        }
    }

    void renderMainMenu()
    {
        BeginDrawing();
        drawBackground();

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        DrawText("PYRAMID SOLITAIRE", sw / 2 - 250, sh / 2 - 300, 50, GOLD);

        Rectangle playBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 180), 300, 60 };
        Rectangle loadBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 90), 300, 60 };
        Rectangle instructBtn = { (float)(sw / 2 - 150), (float)(sh / 2), 300, 60 };
        Rectangle highScoreBtn = { (float)(sw / 2 - 150), (float)(sh / 2 + 90), 300, 60 };
        Rectangle exitBtn = { (float)(sw / 2 - 150), (float)(sh / 2 + 180), 300, 60 };

        DrawRectangleRec(playBtn, DARKGREEN);
        DrawRectangleLinesEx(playBtn, 3, GREEN);
        DrawText("NEW GAME", sw / 2 - 110, sh / 2 - 160, 25, WHITE);

        if (savedGameExists)
        {
            DrawRectangleRec(loadBtn, PURPLE);
            DrawRectangleLinesEx(loadBtn, 3, VIOLET);
            DrawText("LOAD GAME", sw / 2 - 110, sh / 2 - 70, 25, WHITE);
        }
        else
        {
            DrawRectangleRec(loadBtn, DARKGRAY);
            DrawRectangleLinesEx(loadBtn, 3, GRAY);
            DrawText("LOAD GAME", sw / 2 - 110, sh / 2 - 70, 25, GRAY);
        }

        DrawRectangleRec(instructBtn, DARKBLUE);
        DrawRectangleLinesEx(instructBtn, 3, BLUE);
        DrawText("INSTRUCTIONS", sw / 2 - 110, sh / 2 + 20, 25, WHITE);

        DrawRectangleRec(highScoreBtn, ORANGE);
        DrawRectangleLinesEx(highScoreBtn, 3, RED);
        DrawText("HIGH SCORES", sw / 2 - 100, sh / 2 + 110, 25, WHITE);

        DrawRectangleRec(exitBtn, DARKGRAY);
        DrawRectangleLinesEx(exitBtn, 3, BLACK);
        DrawText("EXIT GAME", sw / 2 - 80, sh / 2 + 200, 25, WHITE);

        EndDrawing();
    }

    void renderInstructions()
    {
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
        DrawText("4. Click STOCK to draw (unlimited draws).", sw / 2 - 350, y, 20, WHITE);
        y += spacing + 5;
        DrawText("5. Stock recycles automatically when empty.", sw / 2 - 350, y, 20, WHITE);
        y += spacing + 10;

        DrawText("CONTROLS:", sw / 2 - 350, y, 25, YELLOW);
        y += spacing;
        DrawText("P - Pause/Resume game", sw / 2 - 350, y, 20, WHITE);
        y += spacing;
        DrawText("S - Save current game", sw / 2 - 350, y, 20, WHITE);
        y += spacing;
        DrawText("BACKSPACE - Return to main menu (auto-saves)", sw / 2 - 350, y, 20, WHITE);
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

    void handleMainMenuClick(int mouseX, int mouseY)
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        Rectangle playBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 180), 300, 60 };
        Rectangle loadBtn = { (float)(sw / 2 - 150), (float)(sh / 2 - 90), 300, 60 };
        Rectangle instructBtn = { (float)(sw / 2 - 150), (float)(sh / 2), 300, 60 };
        Rectangle highScoreBtn = { (float)(sw / 2 - 150), (float)(sh / 2 + 90), 300, 60 };
        Rectangle exitBtn = { (float)(sw / 2 - 150), (float)(sh / 2 + 180), 300, 60 };

        if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, playBtn))
        {
            initGame();
        }
        else if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, loadBtn) && savedGameExists)
        {
            loadGame();
        }
        else if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, instructBtn))
        {
            currentState = INSTRUCTIONS;
        }
        else if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, highScoreBtn))
        {
            currentState = HIGH_SCORES;
        }
        else if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, exitBtn))
        {
            CloseWindow();
            exit(0);
        }
    }

    void handleInstructionsClick(int mouseX, int mouseY)
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        Rectangle backBtn = { (float)(sw / 2 - 100), (float)(sh - 120), 200, 50 };
        if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, backBtn))
        {
            currentState = MAIN_MENU;
        }
    }

    void render()
    {
        if (currentState == HIGH_SCORES)
        {
            renderHighScores();
            return;
        }

        if (currentState == MAIN_MENU)
        {
            renderMainMenu();
            return;
        }

        if (currentState == INSTRUCTIONS)
        {
            renderInstructions();
            return;
        }

        BeginDrawing();
        drawBackground();

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        DrawText(TextFormat("Score: %d", score), 20, 20, 25, GOLD);
        DrawText(TextFormat("Moves: %d", moves), sw - 150, 20, 25, YELLOW);

        // Draw pyramid cards (managed by BST)
        for (int i = 0; i < 28; i++)
        {
            PyramidCard& pc = allPyramidCards[i];
            if (pc.card && pc.card->inPlay)
            {
                Rectangle rect = getPyramidCardRect(pc.row, pc.col);
                bool selected = (&pc == selectedPyramid1 || &pc == selectedPyramid2);
                drawCard(pc.card, rect, selected);

                // Draw red line on blocked cards
                if (pc.blocked)
                {
                    DrawRectangle(rect.x, rect.y, rect.width, 5, RED);
                }
            }
        }

        int uiStartY = 150 + 7 * (CARD_HEIGHT / 2 + CARD_SPACING);

        // Draw waste pile
        DrawText("WASTE", 50, uiStartY - 30, 20, WHITE);
        if (currentWasteCard && currentWasteCard->inPlay)
        {
            Rectangle wasteRect = { 50, (float)uiStartY, CARD_WIDTH, CARD_HEIGHT };
            bool selected = (currentWasteCard == selectedCard1 || currentWasteCard == selectedCard2);
            drawCard(currentWasteCard, wasteRect, selected);
        }
        else
        {
            Rectangle emptyRect = { 50, (float)uiStartY, CARD_WIDTH, CARD_HEIGHT };
            DrawRectangleLinesEx(emptyRect, 2, GRAY);
        }

        // Draw stock pile
        stockRect = { 180.0f, (float)uiStartY, (float)CARD_WIDTH, (float)CARD_HEIGHT };
        DrawText("STOCK", 180, uiStartY - 30, 20, WHITE);
        DrawText(TextFormat("(%d)", stockTop + 1), 190, uiStartY + CARD_HEIGHT + 5, 18, LIGHTGRAY);

        if (stockTexture.id != 0 && stockTop >= 0)
        {
            DrawTexturePro(stockTexture,
                { 0, 0, (float)stockTexture.width, (float)stockTexture.height },
                stockRect, { 0, 0 }, 0, WHITE);
        }
        else if (stockTop >= 0)
        {
            DrawRectangleRec(stockRect, BLUE);
            DrawRectangleLinesEx(stockRect, 2, WHITE);
            DrawText("STOCK", stockRect.x + 15, stockRect.y + 55, 18, WHITE);
        }
        else
        {
            DrawRectangleLinesEx(stockRect, 2, GRAY);
            DrawText("RECYCLE", stockRect.x + 8, stockRect.y + 55, 16, GRAY);
        }

        // Draw time
        int totalSeconds = (int)gameTime;
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        DrawText(TextFormat("Time: %02d:%02d:%02d", hours, minutes, seconds), sw / 2 - 80, sh - 30, 25, WHITE);

        // Draw restart button
        Rectangle restartBtn = { (float)(sw - 150), (float)(sh - 60), 120, 50 };
        DrawRectangleRec(restartBtn, MAROON);
        DrawRectangleLinesEx(restartBtn, 2, WHITE);
        DrawText("RESTART", sw - 140, sh - 45, 20, WHITE);

        DrawText("Press S to Save", 20, sh - 30, 20, LIGHTGRAY);

        // Show save message
        if (showSaveMessage && saveMessageTimer > 0)
        {
            DrawText("GAME SAVED!", sw / 2 - 80, 60, 30, GREEN);
        }

        // Draw game over messages
        if (gameWon)
        {
            DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });
            DrawText("YOU WIN!", sw / 2 - 100, sh / 2 - 50, 40, GOLD);
            DrawText(TextFormat("Final Score: %d", score), sw / 2 - 100, sh / 2 + 10, 30, WHITE);
            DrawText("Press BACKSPACE for menu", sw / 2 - 150, sh / 2 + 60, 20, LIGHTGRAY);
        }
        else if (gameLost)
        {
            DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });
            DrawText("NO MOVES LEFT!", sw / 2 - 150, sh / 2 - 50, 40, RED);
            DrawText(TextFormat("Final Score: %d", score), sw / 2 - 100, sh / 2 + 10, 30, WHITE);
            DrawText("Press BACKSPACE for menu", sw / 2 - 150, sh / 2 + 60, 20, LIGHTGRAY);
        }

        if (isPaused)
        {
            DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 150 });
            DrawText("PAUSED", sw / 2 - 80, sh / 2, 40, YELLOW);
            DrawText("Press P to Resume", sw / 2 - 120, sh / 2 + 50, 25, WHITE);
        }

        EndDrawing();
    }

    void update(float deltaTime)
    {
        // Update save message timer
        if (showSaveMessage)
        {
            saveMessageTimer -= deltaTime;
            if (saveMessageTimer <= 0)
            {
                showSaveMessage = false;
            }
        }

        // Handle backspace key
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            if (currentState == PLAYING && !gameWon && !gameLost)
            {
                saveGame();
            }
            currentState = MAIN_MENU;
            isPaused = false;
            checkSavedGame();
            return;
        }

        // Handle high scores screen
        if (currentState == HIGH_SCORES)
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                Vector2 mousePos = GetMousePosition();
                handleHighScoresClick((int)mousePos.x, (int)mousePos.y);
            }
            return;
        }

        // Handle main menu
        if (currentState == MAIN_MENU)
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                Vector2 mousePos = GetMousePosition();
                handleMainMenuClick((int)mousePos.x, (int)mousePos.y);
            }
            return;
        }

        // Handle instructions screen
        if (currentState == INSTRUCTIONS)
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                Vector2 mousePos = GetMousePosition();
                handleInstructionsClick((int)mousePos.x, (int)mousePos.y);
            }
            return;
        }

        // Handle save key
        if (IsKeyPressed(KEY_S) && !gameWon && !gameLost && !isPaused)
        {
            saveGame();
        }

        // Handle pause key
        if (IsKeyPressed(KEY_P) && !gameWon && !gameLost)
        {
            isPaused = !isPaused;
        }

        // Handle paused state
        if (isPaused)
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                Vector2 mousePos = GetMousePosition();
                int sw = GetScreenWidth();
                int sh = GetScreenHeight();
                Rectangle restartBtn = { (float)(sw - 150), (float)(sh - 60), 120, 50 };
                if (CheckCollisionPointRec(mousePos, restartBtn))
                {
                    initGame();
                    return;
                }
            }
            return;
        }

        // Update game time and check lose condition
        if (!gameWon && !gameLost)
        {
            gameTime += deltaTime;

            static float checkTimer = 0.0f;
            checkTimer += deltaTime;
            if (checkTimer >= 0.5f)
            {
                checkLoseCondition();
                checkTimer = 0.0f;
            }
        }

        // Handle mouse clicks
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Vector2 mousePos = GetMousePosition();

            int sw = GetScreenWidth();
            int sh = GetScreenHeight();
            Rectangle restartBtn = { (float)(sw - 150), (float)(sh - 60), 120, 50 };
            if (CheckCollisionPointRec(mousePos, restartBtn))
            {
                initGame();
                return;
            }

            handleMouseClick((int)mousePos.x, (int)mousePos.y);
        }
    }
};

int main()
{
    const int screenWidth = 1400;
    const int screenHeight = 950;

    InitWindow(screenWidth, screenHeight, "Pyramid Solitaire with BST");
    SetTargetFPS(60);

    PyramidSolitaire game;

    while (!WindowShouldClose())
    {
        game.update(GetFrameTime());
        game.render();
    }

    CloseWindow();
    return 0;
}