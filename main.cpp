// To compile: g++ -std=c++11 -lpthread main.cpp

// Mental math game 

#include <iostream> // std::cout, std::cin
#include <thread> // std::thead
#include <queue> // std::queue
#include <chrono> // std::chrono
#include <cstdlib> // rand()

// Expression class
class expression {
private:
    int rightOperand;
    int leftOperand;
    char mathOperator;
    int result;
public:
    // Default constructor, no parameters
    expression() {
        int oprtType = rand() % 4; // generate a number between 0 and 3
        // 0: +, 1: -, 2: *, 3: /
        int tempSwapNumber;
        switch(oprtType) {
            case(0):
                this->mathOperator = '+';
                this->leftOperand = rand() % 100;
                this->rightOperand = rand() % 100;
                break;
            case(1):
                this->mathOperator = '-';
                this->leftOperand = rand() % 100;
                this->rightOperand = rand() % 100;
                if(this->rightOperand > this->leftOperand) {
                    tempSwapNumber = this->leftOperand;
                    this->leftOperand = this->rightOperand;
                    this->rightOperand = tempSwapNumber;
                }
                break;
            case(2):
                this->mathOperator = '*';
                this->leftOperand = rand() % 20;
                this->rightOperand = rand() % 20;
                break;
            case(3):
                this->leftOperand = rand() % 20;    
                this->rightOperand = rand() % 20;
                this->mathOperator = '/';
                break;
        }

    }

    // Constructor for custom expression
    expression(int leftOprnd, char mathOprtr, int rightOprnd) {
        this->leftOperand = leftOprnd;
        this->rightOperand = rightOprnd;
        switch(mathOprtr) {
            case('+'):
            case('-'):
            case('*'):
            case('/'):
                this->mathOperator = mathOprtr;
                break;
            default:
                this->mathOperator = '+'; // if invalid, we will set operator to + anyways
        }
    }
    // Return the result;
    int getResult() const {
        int result;
        switch(this->mathOperator) {
            case('+'):
                result = this->leftOperand + this->rightOperand;
                break;
            case('-'):
                result = this->leftOperand - this->rightOperand;
                break;
            case('*'):
                result = this->leftOperand * this->rightOperand;
                break;
            case('/'):
                result = this->leftOperand / this->rightOperand;
                break;
        }
        return result;
    }

    // function to overload << operator and print out the expression (no equals)
    friend std::ostream &operator<<(std::ostream &out, const expression &expr) {
        out << expr.leftOperand << expr.mathOperator << expr.rightOperand;
        return out;
    }
};

void clearScreen(); // clear the screen
void initializeGame(std::queue<expression> &, int level); // initialize the threads
void addToQueue(std::queue<expression> &, int level); // add expressions into the queue
void answerExpr(std::queue<expression> &); // answer and remove expressions

std::mutex queueLock; 

int main() {
    srand(time(NULL));
    std::queue<expression> exprQueue; // the queue that will hold math expressions

    char userOption;
    int levelOption;


    while(1) {
        std::cout << "=============================================" << std::endl;
        std::cout << "[A.] Start Game" << std::endl;
        std::cout << "[B.] How To Play" << std::endl;
        std::cout << "[C]. Exit Game" << std::endl;
        std::cout << "=============================================" << std::endl;
        std::cout << "Enter choice: ";
        std::cin >> userOption;
        userOption = towlower(userOption);
        switch(userOption) {
            case('a'):
                std::cout << "Choose level (1 to 5) Easiest to Hardest" << std::endl;
                std::cout << "Enter level: ";
                std::cin >> levelOption;
                if(levelOption < 1 || levelOption > 5) {
                    std::cout << "Invalid level entered" << std::endl;
                } else {
                    clearScreen();
                    initializeGame(exprQueue, levelOption);
                }
                break;
            case('b'):
                std::cout << "You will be prompted to answer a math expression such as '5 + 4' " << std::endl;
                std::cout << "Every few seconds another mathematical expression will be added into a queue" << std::endl;
                std::cout << "Once you have 12 math expressions in the queue, you lose" << std::endl;
                std::cout << "If you manage to answer all math questions in 60 seconds without filling up the queue, you win" << std::endl;
                std::cout << "Alternatively, if you manage to empty the queue, you also win" << std::endl;
                break;
            case('c'):
                return 0;

            default:
                clearScreen();
                std::cout << "Invalid input" << std::endl;
                break;
        }
    }
    return 0;
}

// Clear the screen depending on OS
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Initialize the threads in this function
void initializeGame(std::queue<expression> &exprQueue, int level) {
    if(!exprQueue.empty()) {
        
    }
    // Genereate 3 expressions into the queue
    exprQueue.push(expression());
    exprQueue.push(expression());
    exprQueue.push(expression());
    // std::ref() reference wraps so it can be passed by reference into thread functions
    std::thread thread_addToQueue(addToQueue, std::ref(exprQueue), level); // initialize the thread that will be used to add expressions into the queue
    std::thread thread_answerExpr(answerExpr, std::ref(exprQueue)); // initialize the thread used to answer and pop expression from the queue
    thread_addToQueue.join();
    thread_answerExpr.join();
}

// Add an expression into the queue
void addToQueue(std::queue<expression> &exprQueue, int level) {
    auto startTime = std::chrono::steady_clock::now(); // get the current time of the function call
    auto stopTime = startTime + std::chrono::seconds(60); // add 60 seconds to get the time when we should stop the loop
    auto currentTime = std::chrono::steady_clock::now(); // initialize the current time, this will be updated constantly
    auto addTime = currentTime + std::chrono::seconds((-1 * level) + 6); // calculate the number of seconds a new expression should be added to the queue depending on level
    while((exprQueue.size() >= 1 && exprQueue.size() <= 13) && currentTime < stopTime) {
        if(currentTime < addTime) {
            currentTime = std::chrono::steady_clock::now(); 
            continue;
        }
        queueLock.lock(); // lock the mutually exclusively queue to update the queue while the answer works
        exprQueue.push(expression());
        queueLock.unlock(); // unlock the queue
        addTime = currentTime + std::chrono::seconds((-1 * level) + 6); // formula to find the time when the next expression should be added
    }
}

// Prompt the user to enter a problem
void answerExpr(std::queue<expression> &exprQueue) {
    int userInput, correct = 0, incorrect = 0;
    auto startTime = std::chrono::steady_clock::now(); // get the current time of the function call
    auto stopTime = startTime + std::chrono::seconds(60); // add 60 seconds to get the time when we should stop the loop
    auto currentTime = std::chrono::steady_clock::now(); // initialize the current time, this will be updated constantly
    // while currentTime is less than the stopTime, or we are in range of the minimum and maximums
    while((exprQueue.size() >= 1 && exprQueue.size() <= 13) && currentTime < stopTime) {
        std::cout << "Current queue size: " << exprQueue.size() << " /12" << std::endl; 
        std::cout << exprQueue.front() << " = ";
        std::cin >> userInput;
        if(std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore();
        }
        clearScreen();
        // if the user enters the right input
        if(userInput == exprQueue.front().getResult()) {
            queueLock.lock();
            exprQueue.pop();
            queueLock.unlock();
            correct++;
            std::cout << "Correct!" << std::endl;
        } else {
            incorrect++;
            std::cout << "Try again" << std::endl;
        }
        currentTime = std::chrono::steady_clock::now(); // update the current time
    }
    clearScreen();
    if(exprQueue.size() == 0 || std::chrono::steady_clock::now() > currentTime) {
        std::cout << "You win!" << std::endl;
    } else {
        std::cout << "Keep practicing" << std::endl;
    }
    std::cout << "Correct: " << correct << std::endl;
    std::cout << "Incorrect: " << incorrect << std::endl;
}