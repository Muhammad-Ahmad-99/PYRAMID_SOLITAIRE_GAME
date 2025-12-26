#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

enum GameState {
    MAIN_MENU,
    INSTRUCTIONS,
    PLAYING,
    GAME_OVER,
    EXIT_GAME
};

class Card {
public:
    int value;
    int suit;
    bool faceUp;
    bool removed;

    Card* prev;
    Card* next;

    Card* leftChild;
    Card* rightChild;

    Card(int v, int s) {
        value = v;
        suit = s;
        faceUp = false;
        removed = false;
        prev = next = nullptr;
        leftChild = rightChild = nullptr;
    }
};

class DoublyLinkedList {
public:
    Card* head;
    Card* tail;

    DoublyLinkedList() {
        head = tail = nullptr;
    }

    void append(int value, int suit) {
        Card* newCard = new Card(value, suit);
        if (!head)
            head = tail = newCard;
        else {
            tail->next = newCard;
            newCard->prev = tail;
            tail = newCard;
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
    DoublyLinkedList deck;
    Card* stockTop;
    Card* selected;

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
        deck = DoublyLinkedList();
        createDeck();
        deck.shuffle();
        makePyramid();

        selected = nullptr;
        score = 0;
        startTime = time(0);
        gameWon = false;
        gameLost = false;

        state = PLAYING;
    }

    void createDeck() {
        for (int suit = 0; suit < 4; suit++)
            for (int value = 1; value <= 13; value++)
                deck.append(value, suit);
    }

    void makePyramid() {
        Card* curr = deck.head;

        for (int row = 0; row < 7; row++) {
            for (int col = 0; col <= row; col++) {
                curr->faceUp = true;
                curr->leftChild = curr->next;
                curr->rightChild = (curr->next) ? curr->next->next : nullptr;
                curr = curr->next;
            }
        }
        stockTop = curr;
    }

    bool isFree(Card* c) {
        if (!c || c->removed || !c->faceUp)
            return false;
        return (!c->leftChild || c->leftChild->removed) &&
            (!c->rightChild || c->rightChild->removed);
    }

    bool canRemove(Card* a, Card* b) {
        if (!a)
            return false;
        if (!b && a->value == 13 && isFree(a))
            return true;
        if (a && b && isFree(a) && isFree(b) && (a->value + b->value == 13))
            return true;
        return false;
    }

    void drawStock() {
        if (!stockTop) {
            cout << "No more stock!\n";
            return;
        }
        stockTop->faceUp = true;
        cout << "Drew card: " << stockTop->value << endl;
        stockTop = stockTop->next;
    }

    void removePair(Card* a, Card* b = nullptr) {
        if (canRemove(a, b)) {
            a->removed = true;
            if (b)
                b->removed = true;

            score += (b ? 20 : 10);
            checkWin();
        }
        else {
            cout << "Invalid move\n";
        }
    }

    void printPyramid() {
        Card* curr = deck.head;
        for (int row = 0; row < 7; row++) {
            for (int i = 0; i < 6 - row; i++)
                cout << "  ";
            for (int col = 0; col <= row; col++) {
                if (curr->removed)
                    cout << " X ";
                else
                    cout << curr->value << " ";
                curr = curr->next;
            }
            cout << endl;
        }
    }

    Card* findCard(int value) {
        Card* curr = deck.head;
        while (curr) {
            if (curr->value == value && !curr->removed && curr->faceUp)
                return curr;
            curr = curr->next;
        }
        return nullptr;
    }

    void checkWin() {
        Card* curr = deck.head;
        while (curr) {
            if (!curr->removed && curr->faceUp)
                return;
            curr = curr->next;
        }
        gameWon = true;
        state = GAME_OVER;
    }

    void checkLose() {
        if (difftime(time(0), startTime) > 300) {
            gameLost = true;
            state = GAME_OVER;
        }
    }

    void run() {
        char choice;

        while (state != EXIT_GAME) {
            switch (state) {

            case MAIN_MENU:
                cout << "\n=== PYRAMID SOLITAIRE ===\n";
                cout << "1. Start Game\n";
                cout << "2. Instructions\n";
                cout << "3. Exit\n";
                cout << "Choose: ";
                cin >> choice;

                if (choice == '1')
                    initGame();
                else if (choice == '2')
                    state = INSTRUCTIONS;
                else if (choice == '3')
                    state = EXIT_GAME;
                break;

            case INSTRUCTIONS:
                cout << "\nMatch cards that sum to 13.\n";
                cout << "King can be removed alone.\n";
                cout << "Press any key to return.\n";
                cin >> choice;
                state = MAIN_MENU;
                break;

            case PLAYING:
                checkLose();

                cout << "\nTime: " << difftime(time(0), startTime)
                    << "s | Score: " << score << endl;

                printPyramid();

                cout << "d-Draw | r-Remove | q-Quit\n";
                cin >> choice;

                if (choice == 'd')
                    drawStock();
                else if (choice == 'r') {
                    int v1, v2;
                    cin >> v1;
                    Card* c1 = findCard(v1);
                    if (c1 && c1->value == 13) removePair(c1);
                    else {
                        cin >> v2;
                        Card* c2 = findCard(v2);
                        removePair(c1, c2);
                    }
                }
                else if (choice == 'q')
                    state = GAME_OVER;
                break;

            case GAME_OVER:
                cout << (gameWon ? "\nYOU WIN!\n" : "\nGAME OVER\n");
                cout << "Final Score: " << score << endl;
                cout << "r-Restart | m-Menu | e-Exit\n";
                cin >> choice;

                if (choice == 'r')
                    initGame();
                else if (choice == 'm')
                    state = MAIN_MENU;
                else if (choice == 'e')
                    state = EXIT_GAME;
                break;
            }
        }
    }
};

int main() {
    PyramidSolitaire game;
    game.run();
    return 0;
}