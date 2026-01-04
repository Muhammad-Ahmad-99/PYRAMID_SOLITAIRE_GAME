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
};

struct PyramidCard {
    Card* card;
    int row;
    int col;
    bool blocked;

    PyramidCard() {
        card = nullptr;
        row = 0;
        col = 0;
        blocked = true;
    }

    PyramidCard(Card* c, int r, int cl) {
        card = c;
        row = r;
        col = cl;
        blocked = true;
    }
};

class PyramidSolitaire {
public:
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

    GameState currentState;
    bool isPaused;

public:
    PyramidSolitaire() {
        selectedCard1 = selectedCard2 = nullptr;
        selectedPyramid1 = selectedPyramid2 = nullptr;
        currentWasteCard = nullptr;
        score = 0;
        moves = 0;
        gameTime = 0.0f;
        gameWon = false;
        gameLost = false;
        stockTop = -1;
        wasteTop = -1;
        currentState = MAIN_MENU;
        isPaused = false;

        initGame();
    }

    void initGame() {
        // Reset game
        score = moves = 0;
        gameTime = 0;
        gameWon = gameLost = false;
        stockTop = wasteTop = -1;

        selectedCard1 = selectedCard2 = nullptr;
        selectedPyramid1 = selectedPyramid2 = nullptr;
        currentWasteCard = nullptr;

        // Create deck
        int index = 0;
        for (int s = 0; s < 4; s++) {
            for (int v = 1; v <= 13; v++) {
                allCards[index] = Card(v, s, index);
                index++;
            }
        }

        // Shuffle deck
        srand(time(nullptr));
        for (int i = 51; i > 0; i--) {
            int j = rand() % (i + 1);
            swap(allCards[i], allCards[j]);
            allCards[i].position = i;
        }

        // Create pyramid
        int cardIndex = 0;
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col <= row; col++) {
                allCards[cardIndex].faceUp = true;
                allPyramidCards[cardIndex] = PyramidCard(&allCards[cardIndex], row, col);
                cardIndex++;
            }
        }

        // Fill stock
        for (int i = 28; i < 52; i++) {
            stockArray[++stockTop] = &allCards[i];
        }

        updateBlockedStatus();
        currentState = PLAYING;
    }

    void updateBlockedStatus() {
        for (int i = 0; i < 28; i++) {
            PyramidCard& pc = allPyramidCards[i];
            if (pc.row == 6) 
                pc.blocked = false;
            else {
                bool leftBlocked = false;
                bool rightBlocked = false;
                int leftIdx = ((pc.row + 1) * (pc.row + 2)) / 2 + pc.col;
                int rightIdx = leftIdx + 1;

                if (allPyramidCards[leftIdx].card->inPlay) 
                    leftBlocked = true;
                if (allPyramidCards[rightIdx].card->inPlay) 
                    rightBlocked = true;

                pc.blocked = leftBlocked || rightBlocked;
            }
        }
    }

    bool isCardFree(PyramidCard* pc) {
        if (!pc || !pc->card || !pc->card->inPlay) 
            return false;
        return !pc->blocked;
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
            score += 10;
            moves++;
            selectedCard1 = nullptr;
            selectedPyramid1 = nullptr;
            updateBlockedStatus();
            checkWinCondition();
            return;
        }

        if (selectedCard1 && selectedCard2 && isValidMove(selectedCard1, selectedCard2)) {
            selectedCard1->inPlay = false;
            selectedCard2->inPlay = false;
            score += 20;
            moves++;
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
        if (!card || !card->inPlay) 
            return;
        if (pc && !isCardFree(pc)) 
            return;

        if (isKing(card)) {
            selectedCard1 = card;
            selectedPyramid1 = pc;
            selectedCard2 = nullptr;
            selectedPyramid2 = nullptr;
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
        gameWon = true;
        for (int i = 0; i < 28; i++) {
            if (allPyramidCards[i].card && allPyramidCards[i].card->inPlay) {
                gameWon = false;
                break;
            }
        }
    }

    void checkLoseCondition() {
        bool hasMoves = false;

        // Check free pyramid cards
        for (int i = 0; i < 28; i++) {
            if (isCardFree(&allPyramidCards[i])) 
                hasMoves = true;
        }

        // Check waste card
        if (currentWasteCard && currentWasteCard->inPlay) 
            hasMoves = true;

        // Check pairs that sum to 13
        Card* freeCards[30];
        int freeCount = 0;
        for (int i = 0; i < 28; i++) {
            if (isCardFree(&allPyramidCards[i])) 
                freeCards[freeCount++] = allPyramidCards[i].card;
        }
        if (currentWasteCard && currentWasteCard->inPlay) 
            freeCards[freeCount++] = currentWasteCard;

        for (int i = 0; i < freeCount; i++) {
            if (isKing(freeCards[i])) 
                return;
        }
        for (int i = 0; i < freeCount; i++) {
            for (int j = i + 1; j < freeCount; j++) {
                if (isValidMove(freeCards[i], freeCards[j])) 
                    return;
            }
        }

        if (stockTop >= 0) 
            return;

        gameLost = true;
    }

    void update(float deltaTime) {
        gameTime += deltaTime;
        checkLoseCondition();
    }
};

// -------------------- CONSOLE DISPLAY --------------------
void printPyramid(PyramidSolitaire& game) {
    cout << "\nPYRAMID:\n\n";
    int rowStart = 0;
    for (int row = 0; row < 7; row++) {
        for (int s = 0; s < 6 - row; s++) cout << "   ";
        for (int col = 0; col <= row; col++) {
            PyramidCard& pc = game.allPyramidCards[rowStart + col];
            if (!pc.card->inPlay) cout << " XX ";
            else if (pc.card == game.selectedCard1 || pc.card == game.selectedCard2) {
                cout << "[";
                if (pc.card->value == 13) cout << "K";
                else cout << pc.card->value;
                cout << "]";
            }
            else {
                if (pc.card->value == 13) cout << " K ";
                else cout << " " << pc.card->value << " ";
            }
        }
        cout << "\n\n";
        rowStart += row + 1;
    }
}

void printStockAndWaste(PyramidSolitaire& game) {
    cout << "STOCK: ";
    if (game.stockTop >= 0) cout << game.stockTop + 1 << " cards";
    else cout << "empty";
    cout << " | WASTE: ";
    if (game.currentWasteCard) {
        if (game.currentWasteCard->value == 13) cout << "K";
        else cout << game.currentWasteCard->value;
    }
    else cout << "empty";
    cout << "\n";
}

void printGameStatus(PyramidSolitaire& game) {
    cout << "Score: " << game.score << " | Moves: " << game.moves
        << " | Time: " << (int)game.gameTime << "s\n";
}

int main() {
    PyramidSolitaire game;
    char choice;

    while (!game.gameWon && !game.gameLost) {
        game.update(1.0f);
        system("cls"); 

        printGameStatus(game);
        printPyramid(game);
        printStockAndWaste(game);

        cout << "\nChoose action:\n";
        cout << "1. Draw from stock\n";
        cout << "2. Pick pyramid card by row and column (e.g. 6 3)\n";
        cout << "Choice (1/2): ";
        cin >> choice;

        if (choice == '1') {
            game.drawCardFromStock();
        }
        else if (choice == '2') {
            int r, c;
            cout << "Enter row (0-6) and column: ";
            cin >> r >> c;
            if (r >= 0 && r <= 6 && c >= 0 && c <= r) {
                int idx = (r * (r + 1)) / 2 + c;
                game.selectCard(game.allPyramidCards[idx].card, &game.allPyramidCards[idx]);
            }
        }
    }

    cout << "\nFinal Status:\n";
    printGameStatus(game);
    if (game.gameWon) 
        cout << "YOU WIN!\n";
    else if (game.gameLost) 
        cout << "NO MOVES LEFT!\n";

    return 0;
}
