#include<iostream>
#include<vector>

using Literal = int;
using Clause = std::vector<Literal>;
using NormalForm = std::vector<Clause>;

int varCount = 0;
NormalForm cnf;

int p(int i);
int q(int i);
void R(int i, int j);
void nJ(int i, int j);
void clause(const Clause &c);
void printDimacs();

int p(int i) {
    return 2*i+1;
}

int q(int i) {
    return 2*i+2;
}

void R(int i, int j) {
    clause({p(i), q(i), -p(i+1)});
    clause({p(i), -q(i), p(i+1)});
    clause({-p(i), q(i), p(i+1)});
    clause({-p(i), -q(i), -p(i+1)});
    clause({q(i), q(i+1)});
    clause({-q(i), -q(i+1)});
}

void nJ(int i, int j) {
    clause({p(i), p(j), q(i), q(j)});
    clause({-p(i), -p(j), q(i), q(j)});
    clause({p(i), p(j), -q(i), -q(j)});
    clause({-p(i), -p(j), -q(i), -q(j)});
}

void clause(const Clause& c) {
    cnf.push_back(c);
    for(auto& literal : c)
        varCount = std::max(varCount, std::abs(literal));
}

void printDimacs() {
    std::cout << "p cnf " << varCount << " " << cnf.size() << std::endl;
    for(auto& clause : cnf) {
        for(auto& literal : clause) {
            std::cout << literal << " ";
        }
        std::cout << "0" << std::endl;
    }
}

int main() {
    R(0, 1);
    R(1, 2);
    R(2, 3);
    R(3, 4);
    nJ(0, 4);

    printDimacs();
}