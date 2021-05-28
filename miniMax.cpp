#include <iostream>


int max(int in, int in2) {
    return in > in2 ? in : in2;
}

int min(int in, int in2) {
    return in < in2 ? in : in2;
}

int miniMax(int * values, int index, int depth, bool maximizing, int alpha, int beta, int * counter) {
    if(depth == 0) {
        (*counter)++;
        return values[index];
    }

    if(maximizing) {
        int cValue = -1000;
        for(int i = 0; i < 2; i++) {
            cValue = max(cValue, miniMax(values, index * 2 + i, depth - 1, false, alpha, beta, counter));
            alpha = max(alpha, cValue);
            if(beta <= alpha) {
                return cValue;
            }
        }
        return cValue;
    } else {
        int cValue = 1000;
        for(int i = 0; i < 2; i++) {
            cValue = min(cValue, miniMax(values, index * 2 + i, depth - 1, true, alpha, beta, counter));
            beta = min(beta, cValue);
            if(beta <= alpha) {
                return cValue;
            }
        }
        return cValue;
    }
}

int main() {
    int values[16] = {5, -1, 4, 3, -2, -5, 9, 8, 6, 1, -4, 2, 4, 7, 3, -3};
    int counter = 0;
    int index = 0;
    std::cout << miniMax(values, index, 4, true, -1000, 1000, &counter) << " " << counter << std::endl;;
}