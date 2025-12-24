#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

class Card {
public:
    int value;      // 1–13
    int suit;       // 0–3
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
        prev = next = NULL;
        leftChild = rightChild = NULL;
    }
};
class DoublyLinkedList {
public:
    Card* head;
    Card* tail;

    DoublyLinkedList() {
        head = tail = NULL;
    }

    void append(int value, int suit) {
        Card* c = new Card(value, suit);

        if (!head) {
            head = tail = c;
        } else {
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
    Card* stockTop;       // Remaining cards after pyramid
    Card* selected;       // Selected card

public:
    PyramidSolitaire() {
        createDeck();
        deck.shuffle();
        initPyramid();
    }

    void createDeck() {
        for (int suit = 0; suit < 4; suit++) {
            for (int value = 1; value <= 13; value++) {
                deck.append(value, suit);
            }
        }
    }

    void initPyramid() {
        Card* curr = deck.head;

        for (int row = 0; row < 7; row++) {
            for (int col = 0; col <= row; col++) {
                curr->faceUp = true;

                // Set pyramid children
                curr->leftChild  = curr->next;
                curr->rightChild = (curr->next) ? curr->next->next : NULL;

                curr = curr->next;
            }
        }

        stockTop = curr; // Remaining cards
    }

    bool isFree(Card* c) {
        if (!c || c->removed || !c->faceUp) return false;

        return (!c->leftChild || c->leftChild->removed) &&
               (!c->rightChild || c->rightChild->removed);
    }

    bool canRemove(Card* a, Card* b) {
        return a && b && isFree(a) && isFree(b) &&
               (a->value + b->value == 13);
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

    void removePair(Card* a, Card* b) {
        if (canRemove(a, b)) {
            a->removed = true;
            b->removed = true;
            cout << "Removed pair\n";
        } else {
            cout << "Invalid move\n";
        }
    }

    void printPyramid() {
        Card* curr = deck.head;
        int index = 0;

        for (int row = 0; row < 7; row++) {
            for (int i = 0; i < 6 - row; i++) cout << " ";

            for (int col = 0; col <= row; col++) {
                if (curr->removed) cout << " X ";
                else cout << curr->value << " ";
                curr = curr->next;
                index++;
            }
            cout << endl;
        }
    }
};

int main() {
    PyramidSolitaire game;

    cout << "Initial Pyramid:\n";
    game.printPyramid();

    cout << "\nDrawing from stock...\n";
    game.drawStock();

    return 0;
}


