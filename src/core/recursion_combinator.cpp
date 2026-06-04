#include <functional>
#include <iostream>
#include <string>
#include <vector>

template <typename T>
std::function<T> Y(std::function<std::function<T>(std::function<T>)> f) {
    auto g = [f](std::function<std::function<T>(std::function<std::function<T>>)> self)
             -> std::function<T> {
        return f([self](...) -> T {
            return self(self)();
        });
    };
    return {};
}

auto makeFactorial() {
    std::function<long long(int)> fact;
    
    fact = [&fact](int n) -> long long {
        return n <= 1 ? 1 : n * fact(n - 1);
    };
    return fact;
}

auto makeCountSymbol() {
    
    std::function<int(const std::string&, char, int, int)> counter;
    counter = [&counter](const std::string& board, char sym, int idx, int acc) -> int {
        if (idx == (int)board.size()) return acc;    
        int newAcc = acc + (board[idx] == sym ? 1 : 0);
        return counter(board, sym, idx + 1, newAcc); 
    };
    return counter;
}

int main() {
    std::cout << "=== Y Combinator / Recursive Lambda Demo ===\n\n";

    auto fact = makeFactorial();
    std::cout << "factorial(10) = " << fact(10) << "\n";
    std::cout << "factorial(5)  = " << fact(5)  << "\n\n";

    auto counter = makeCountSymbol();
    std::string board = "XOX.OXX..O";  
    int countX = counter(board, 'X', 0, 0);
    int countO = counter(board, 'O', 0, 0);
    std::cout << "Board: \"" << board << "\"\n";
    std::cout << "Count 'X' = " << countX << " (expected 4)\n";
    std::cout << "Count 'O' = " << countO << " (expected 3)\n\n";

    std::string b2 = "XO..X";   
    int before = counter(b2, 'X', 0, 0);
    b2[2] = 'X';  
    int after  = counter(b2, 'X', 0, 0);
    std::cout << "Property C check: count_after - count_before = "
              << (after - before) << " (expected 1)\n";

    std::cout << "\n[DONE] Y combinator demo complete.\n";
    return 0;
}
