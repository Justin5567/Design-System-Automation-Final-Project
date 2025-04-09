#ifndef DPLL_H
#define DPLL_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>


class Clause {
    public:
        std::vector<int> literals; // List of literals in the clause
    
        void addLiteral(int literal) {
            literals.push_back(literal);
        }
    
        void printClause() const {
            for (int literal : literals) {
                std::cout << literal << " ";
            }
            std::cout << "0" << std::endl; // End of clause
        }
    };

void clausesIsEmpty();

void clausesHaveEmptyClause();

bool dpll(const std::vector<Clause>& clauses);
    

#endif