#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

class Card {
public:
    int value;      // 1-13
    int suit;       // 0-3
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

public:
    PyramidSolitaire() {
        createDeck();
        deck.shuffle();
        makePyramid();
        selected = nullptr;

        score = 0;
        startTime = time(0);
        gameWon = false;
        gameLost = false;
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
            cout << "No more stock cards!\n";
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

            if (!b) {
                score += 10;
                cout << "Removed King " << a->value << endl;
            }
            else {
                score += 20;
                cout << "Removed pair " << a->value << "+" << b->value << endl;
            }

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
        cout << "Card not found or not free!\n";
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
    }

    void checkLose() {
        int timeGap = difftime(time(0), startTime);
        if (timeGap > 300) { 
            gameLost = true;
        }
    }

    void play() {
        char choice;

        while (true) {
            checkLose();

            int timeGap = difftime(time(0), startTime);
            cout << "\nTime: " << timeGap << "s | Score: " << score << endl;

            if (gameWon) {
                cout << "\nYOU WIN!" << endl;
                cout << "Final Score : " << score << endl;
                break;
            }

            if (gameLost) {
                cout << "\nGAME OVER (Time Up)" << endl;
                cout << "Final Score : " << score << endl;
                break;
            }

            cout << "\nPyramid:\n";
            printPyramid();

            cout << "\nOptions:\n";
            cout << "d - Draw stock\n";
            cout << "r - Remove card/pair\n";
            cout << "q - Quit\n";
            cout << "Choose: ";
            cin >> choice;

            if (choice == 'd' || choice == 'D') {
                drawStock();
            }
            else if (choice == 'r' || choice == 'R') {
                int v1, v2;
                cout << "Enter first card value: ";
                cin >> v1;
                Card* c1 = findCard(v1);

                if (c1 && c1->value == 13) {
                    removePair(c1);
                }
                else {
                    cout << "Enter second card value: ";
                    cin >> v2;
                    Card* c2 = findCard(v2);
                    removePair(c1, c2);
                }
            }
            else if (choice == 'q' || choice == 'Q') {
                break;
            }
            else {
                cout << "Invalid option\n";
            }
        }
    }
};

int main() {
    cout << "Welcome to Pyramid Solitaire!\n";
    PyramidSolitaire game;
    game.play();
    return 0;
}
