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
        Card* c = new Card(value, suit);
        if (!head) head = tail = c;
        else {
            tail->next = c;
            c->prev = tail;
            tail = c;
        }
    }

    void shuffle() {
        srand(time(0));
        for (Card* a = head; a; a = a->next) {
            Card* b = head;
            int steps = rand() % 52;
            while (steps-- && b->next) b = b->next;
            swap(a->value, b->value);
            swap(a->suit, b->suit);
        }
    }

    void printDeck() {
        for (Card* c = head; c; c = c->next) {
            cout << c->value << "-" << c->suit << " ";
        }
        cout << endl;
    }
};

class PyramidSolitaire {
private:
    DoublyLinkedList deck;
    Card* stockTop;    // Remaining cards after pyramid
    Card* selected;    // Selected card

public:
    PyramidSolitaire() {
        createDeck();
        deck.shuffle();
        initPyramid();
        selected = nullptr;
    }

    void createDeck() {
        for (int suit = 0; suit < 4; suit++)
            for (int value = 1; value <= 13; value++)
                deck.append(value, suit);
    }

    void initPyramid() {
        Card* curr = deck.head;

        for (int row = 0; row < 7; row++) {
            for (int col = 0; col <= row; col++) {
                curr->faceUp = true;

                // Set pyramid children (simple linking)
                curr->leftChild = curr->next;
                curr->rightChild = (curr->next) ? curr->next->next : nullptr;

                curr = curr->next;
            }
        }

        stockTop = curr; // Remaining cards are stock
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
        // King removal alone
        if (!b && a->value == 13 && isFree(a)) 
            return true;
        // Pair sum = 13
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
            cout << "Removed ";
            if (!b) 
                cout << "King " << a->value << endl;
            else 
                cout << "pair " << a->value << "+" << b->value << endl;
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

    void play() {
        char choice;
        while (true) {
            cout << "\nPyramid:\n";
            printPyramid();
            cout << "\nOptions:\n  1. d to Draw stock \n  2. r to Remove card/pair \n  3. q to Quit \nChoose: ";
            cin >> choice;

            if (choice == 'd' || choice == 'D') {
                drawStock();
            }
            else if (choice == 'r' || choice == 'R') {
                int v1, v2 = 0;
                cout << "Enter first card value to remove: ";
                cin >> v1;
                Card* c1 = findCard(v1);

                if (c1 && c1->value == 13) {
                    removePair(c1); // King
                }
                else {
                    cout << "Enter second card value to remove: ";
                    cin >> v2;
                    Card* c2 = findCard(v2);
                    removePair(c1, c2);
                }
            }
            else if (choice == 'q' || choice == 'Q') 
                break;
            else cout << "Invalid option\n";
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
};

int main() {
    PyramidSolitaire game;
    cout << "Welcome to Pyramid Solitaire!\n";
    game.play();
    return 0;
}
