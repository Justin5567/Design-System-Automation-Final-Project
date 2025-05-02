#ifndef DPLL_H
#define DPLL_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>


class Clause {
    public:
        std::unordered_map<int,int> literals; // List of literals in the clause
        int watch1;
        int watch2;
        
        void addLiteral(int literal) {
            // literals.push_back(literal);
            literals[literal] = 1;
        }
    
        void printClause() const {
            for (auto & literal: literals) {
                std::cout << literal.first << " ";
            }
            std::cout<<std::endl;
            // std::cout << "0" << std::endl; // End of clause
        }
    };

void clausesIsEmpty();

void clausesHaveEmptyClause();

bool dpll(const std::vector<Clause>& clauses);
    

#endif