#include <iostream>
#include <ctime>
#include <cstdlib>

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

// Stack Node class
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
    }

    ~PyramidSolitaire() {
        clearPyramid();
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
        cout << "Game initialized with Stack-based structures!\n";
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

        // Shuffle array
        for (int i = 51; i > 0; i--) {
            int j = rand() % (i + 1);
            swap(tempDeck[i], tempDeck[j]);
        }

        // Update allCards
        for (int i = 0; i < 52; i++) {
            allCards[i] = tempDeck[i];
        }

        // Push back to deck stack
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
            // Rebuild stock from backup using Stack
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

            cout << "Stock reset from backup!\n";
        }

        Card* card = stock.pop(); // LIFO operation
        card->faceUp = true;
        currentWasteCard = card;
        wasteHistory.push(card);  // LIFO operation
        cout << "Drew card: " << card->value << "\n";
    }

    void removeCards() {
        if (selectedCard1 && isKing(selectedCard1)) {
            cout << "King removed! +10 points\n";
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
            cout << "Valid pair removed! +20 points\n";
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
            cout << "Invalid pair!\n";
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
            cout << "Card is blocked!\n";
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
            cout << "First card selected: " << card->value << "\n";
        }
        else if (selectedCard1 == card) {
            selectedCard1 = nullptr;
            selectedNode1 = nullptr;
            cout << "Card deselected\n";
        }
        else {
            selectedCard2 = card;
            selectedNode2 = node;
            cout << "Second card selected: " << card->value << "\n";
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
        // Using Stack to collect free cards
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

        // Check for Kings
        StackNode<Card*>* checkNode = freeCards.getTop();
        while (checkNode) {
            if (isKing(checkNode->data))
                return;
            checkNode = checkNode->next;
        }

        // Check for valid pairs
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

    void update(float deltaTime) {
        if (currentState == PLAYING && !isPaused && !gameWon && !gameLost) {
            gameTime += deltaTime;
            checkLoseCondition();
        }
    }

    void displayMainMenu() {
        cout << "\n   PYRAMID SOLITAIRE (STACK)    \n";
        cout << "1. NEW GAME\n";
        cout << "2. INSTRUCTIONS\n";
        cout << "3. EXIT\n";
        cout << "Choice: ";
    }

    void displayInstructions() {
        cout << "\n═══ HOW TO PLAY ═══\n";
        cout << "• Pair cards that sum to 13\n";
        cout << "• Kings (13) removed alone\n";
        cout << "• King: +10 pts | Pair: +20 pts\n";
        cout << "\nStack-based structures:\n";
        cout << "• Deck (Stack)\n";
        cout << "• Stock (Stack - LIFO)\n";
        cout << "• Waste History (Stack - LIFO)\n";
        cout << "\nPress Enter...";
        cin.ignore();
        cin.get();
    }

    void printPyramid() {
        cout << "\nPYRAMID:\n\n";
        for (int row = 0; row < 7; row++) {
            for (int s = 0; s < 6 - row; s++)
                cout << "   ";

            PyramidNode* current = pyramidRows[row];
            while (current) {
                if (!current->card->inPlay) {
                    cout << " XX ";
                }
                else if (current == selectedNode1 || current == selectedNode2) {
                    cout << "[";
                    if (current->card->value == 13) cout << "K";
                    else cout << current->card->value;
                    cout << "]";
                }
                else {
                    if (current->card->value == 13) cout << " K ";
                    else if (current->card->value < 10) cout << " " << current->card->value << " ";
                    else cout << current->card->value << " ";
                }
                current = current->nextInRow;
            }
            cout << "\n\n";
        }
    }

    void printStockAndWaste() {
        cout << "STOCK: ";
        if (!stock.isEmpty())
            cout << stock.getSize() << " cards";
        else
            cout << "empty";

        cout << " | WASTE: ";
        if (currentWasteCard && currentWasteCard->inPlay) {
            if (currentWasteCard->value == 13)
                cout << "K";
            else
                cout << currentWasteCard->value;

            if (currentWasteCard == selectedCard1 || currentWasteCard == selectedCard2)
                cout << " [SELECTED]";
        }
        else
            cout << "empty";
        cout << "\n";
    }

    void printGameStatus() {
        cout << "Score: " << score << " | Moves: " << moves
            << " | Time: " << (int)gameTime << "s\n";
        cout << "Stack Info: Deck=" << deck.getSize()
            << " Stock=" << stock.getSize()
            << " Waste=" << wasteHistory.getSize() << "\n";
    }

    void run() {
        char choice;

        while (true) {
            if (currentState == MAIN_MENU) {
                system("cls");
                displayMainMenu();
                cin >> choice;
                cin.ignore();

                if (choice == '1') {
                    initGame();
                }
                else if (choice == '2') {
                    displayInstructions();
                }
                else if (choice == '3') {
                    cout << "Thanks for playing!\n";
                    break;
                }
            }
            else if (currentState == PLAYING) {
                if (gameWon || gameLost) {
                    system("cls");
                    printGameStatus();
                    if (gameWon)
                        cout << "\n★★★ YOU WIN! ★★★\n";
                    else if (gameLost)
                        cout << "\n✖ NO MOVES LEFT! ✖\n";

                    cout << "\nPress Enter to return to menu...";
                    cin.get();
                    currentState = MAIN_MENU;
                    continue;
                }

                update(1.0f);
                system("cls");

                printGameStatus();
                printPyramid();
                printStockAndWaste();

                cout << "\nActions:\n";
                cout << "1. Draw from stock (Stack pop)\n";
                cout << "2. Select pyramid card (row col)\n";
                cout << "3. Select waste card\n";
                cout << "4. Back to menu\n";
                cout << "Choice: ";
                cin >> choice;

                if (choice == '1') {
                    drawCardFromStock();
                    cin.ignore();
                    cin.get();
                }
                else if (choice == '2') {
                    int r, c;
                    cout << "Enter row (0-6) and column: ";
                    cin >> r >> c;
                    if (r >= 0 && r <= 6 && c >= 0 && c <= r) {
                        PyramidNode* node = pyramidRows[r];
                        for (int i = 0; i < c && node; i++) {
                            node = node->nextInRow;
                        }
                        if (node) {
                            selectCard(node->card, node);
                        }
                    }
                    cin.ignore();
                    cin.get();
                }
                else if (choice == '3') {
                    if (currentWasteCard) {
                        selectCard(currentWasteCard, nullptr);
                    }
                    cin.ignore();
                    cin.get();
                }
                else if (choice == '4') {
                    currentState = MAIN_MENU;
                }
            }
        }
    }
};

int main() {

    cout << "\n Pyramid Solitaire \n";
    cout << "\nPress Enter to start...";
    cin.get();

    PyramidSolitaire game;
    game.run();

    return 0;
}