#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include "dpll.h"
#include <chrono>

// void eliminatePureLiterals(std::vector<Clause> & clauses){
//     std::unordered_map<int, int> literalCount;
//     for(const auto& clause: clauses){
//         for(const auto literal: clause.literals){
//             literalCount[literal.first]++;
//             // std::cout << literal.first << " ";
//         }
//     }
//     for(const auto& [literal,times]:literalCount){
//         if(literalCount.count(-literal)==0){
//             std::vector<Clause> newClauses;
//             for(const auto& clause : clauses){
//                 if(!clause.literals.count(literal)){
//                     newClauses.push_back(clause);
//                 }
//             }
//             clauses = newClauses;
//             eliminatePureLiterals(clauses);
//             break;
//         }
//     }
// }

void eliminatePureLiterals(std::vector<Clause>& clauses) {
    std::unordered_map<int, int> literalCount;
    
    for (const auto& clause : clauses) {
        for (const auto& literal : clause.literals) {
            literalCount[literal.first]++;
        }
    }

    std::unordered_map<int, bool> isPure;
    for (const auto& [literal, _] : literalCount) {
        if (literalCount.count(-literal) == 0) {
            isPure[literal] = true;
        }
    }

    if (isPure.empty()) return; 

    std::vector<Clause> newClauses;
    for (const auto& clause : clauses) {
        bool keep = true;
        for (const auto& literal : clause.literals) {
            if (isPure.count(literal.first)) {
                keep = false;
                break;
            }
        }
        if (keep) {
            newClauses.push_back(clause);
        }
    }

    clauses = std::move(newClauses);

    eliminatePureLiterals(clauses);
}


// basic version pick the first literal in the clauses
int chooseLiteral(const std::vector<Clause> & clauses){
    for(const auto& clause : clauses){
        return clause.literals.begin()->first;
    }
    return 0;
}

// count and sort literals from largest number of occurences to smallest
void DLIS(const std::vector<Clause> & clauses){
    std::unordered_map<int, int> literalCount;

    // 1. Count each literal
    for (const auto& clause : clauses) {
        for (const auto& literal : clause.literals) {
            literalCount[literal.first]++;
        }
    }

    // 2. Move counts into a vector for sorting
    std::vector<std::pair<int, int>> countsVec;
    for (const auto& [literal, count] : literalCount) {
        countsVec.push_back({literal, count});
    }

    // 3. Sort descending by count
    std::sort(countsVec.begin(), countsVec.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        return a.second > b.second; // larger counts first
    });

    // 4. (Optional) Print them out
    // for (const auto& [literal, count] : countsVec) {
    //     std::cout << "Literal " << literal << " appears " << count << " times\n";
    // }
}


// set the picking literal as true (false if is negative)
// simplify the whole clauses with new update value
bool unitPropagate(std::vector<Clause>& clauses, int literal, std::unordered_map<int, bool>& assignment){
    assignment[std::abs(literal)] = (literal > 0);
    std::vector<Clause> newClauses;
    for(const auto& clause: clauses){
        if(clause.literals.count(literal)) continue;
        if(clause.literals.count(-literal)){
            Clause newClause = clause;
            newClause.literals.erase(-literal);
            if(newClause.literals.empty()) return false;
            newClauses.push_back(newClause);
        }
        else{
            newClauses.push_back(clause);
        }
    }
    clauses = newClauses;
    return true;
}

bool dpll(std::vector<Clause>& clauses, std::unordered_map<int, bool>& assignment){
    // std::cout<<"vector size"<<clauses.size()<<std::endl;
    // for (const Clause& clause : clauses) {
    //     clause.printClause();
    // }
    bool updated = true;
    while(updated){
        updated = false;
        for(const auto& clause : clauses){
            if(clause.literals.size()==1){
                int unit = clause.literals.begin()->first;
                if(!unitPropagate(clauses, unit, assignment)) return false;
                updated = true;
                break;
            }
        }
    }
    // std::cout<<"vector size"<<clauses.size()<<std::endl;
    // for (const Clause& clause : clauses) {
    //     clause.printClause();
    // }
    // std::cout<<"------------------"<<std::endl;
    // eliminatePureLiterals(clauses);

    DLIS(clauses);
    int literal = chooseLiteral(clauses);
    // std::cout<<"[Choose Literal] "<<literal<<std::endl;
    
    if(clauses.empty()) return true;
    for(const auto& clause : clauses){
        if(clause.literals.empty()) return false;
    }
    std::vector<Clause> copy1 = clauses;
    std::vector<Clause> copy2 = clauses;
    std::unordered_map<int, bool> assignment_copy1 = assignment;
    std::unordered_map<int, bool> assignment_copy2 = assignment;
    if(unitPropagate(copy1, literal, assignment_copy1) && dpll(copy1, assignment_copy1)) {
        // std::cout<<"enter copy1"<<std::endl;
        assignment = std::move(assignment_copy1);
        return true;
    }
    if(unitPropagate(copy2, -literal, assignment_copy2) && dpll(copy2, assignment_copy2)) {
        // std::cout<<"enter copy2"<<std::endl;
        assignment = std::move(assignment_copy2);
        return true;
    }
    
    return false;

}

int main() {
    // std::ifstream file("./uf20-91/uf20-0965.cnf"); // Open the file named "uf20-91"
    std::ifstream file("./test2.cnf"); // unsat test
    if (!file) { 
        std::cerr << "Error: Could not open the file 'uf20-91'." << std::endl;
        return 1;
    }

    std::vector<Clause> clauses; 
    std::string line;

    while (std::getline(file, line)) { // Read the file line by line
        if (line.empty() || line[0] == 'c' || line[0] == 'p' || line[0] == '%' || line[0] == '0') {
            // Skip comments (lines starting with 'c') and problem line (starting with 'p')
            continue;
        }

        std::istringstream iss(line);
        Clause clause;
        int literal;

        while (iss >> literal) {
            if (literal == 0) {
                break; // End of clause
            }
            clause.addLiteral(literal);
        }
        clauses.push_back(clause); 
    }

    file.close(); // Close the file

    // Print all clauses for verification
    // for (const Clause& clause : clauses) {
    //     clause.printClause();
    // }

    using namespace std:: chrono;
    auto start = high_resolution_clock::now();

    std::unordered_map<int, bool> assignment;

    
    bool ans = false;
    ans = dpll(clauses,assignment);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    if(ans){
        std::cout<<"SAT Solve"<<std::endl;
        for (const auto& it : assignment) {
            int var = it.first;
            bool value = it.second;
            std::cout << var << " = " << (value ? "TRUE" : "FALSE") << std::endl;
        }
    }
    else{
        std::cout<<"SAT Not Solve"<<std::endl;
    }
    std::cout << "Execution Time: " << duration.count() << " microseconds" << std::endl;

    return 0;
}