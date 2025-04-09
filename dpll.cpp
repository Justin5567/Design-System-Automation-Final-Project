#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <algorithm>
#include "dpll.h"


// void unitPropagation(const std::vector<Clause>& clauses, int selLiteral){
//     for(const auto clause:clauses){
//         auto check = std::find(clause.literals.begin(),clause.literals.end(),selLiteral);
//         auto check_neg = td::find(clause.literals.begin(),clause.literals.end(),-1 * selLiteral);
//         if(check!=clause.literals.end()){ //exist
//             continue;
//         }
//         if(check_neg!=clause.literals.end()){ // neg term exist remove it
//             std::vector
//         }
//     }
// }

// void pureEliminate(const std::vector<Clause>& clauses){
//     // do the count first
//     std::map<int,int> literalCount;
//     for(const auto clause: clauses){
//         for(const auto literal: clause.literals){
//             literalCount[literal]++;
//         }
//     }

//     //print to see how many literal
//     // for(const auto&[key,value]:literalCount){
//     //     std::cout<<"literal: "<<key<<",  value: "<<value<<std::endl;
//     // }

//     // if there is only positive/negative remove the clause
//     std::vector<Clause> tmp_clauses;
//     for(const auto &[key, value]:literalCount){
//         if(literalCount.count(-key)==0){ // key is pure

//             //traverse clauses again
//             for(const auto tmp:clauses){
//                 if (find(tmp.literals.begin(), tmp.literals.end(), key) == tmp.literals.end()) {
//                     tmp_clauses.push_back(tmp);
//                 }
//             }
//             pureEliminate(tmp_clauses);
//             break;
//         }
        
//     }

// }

// void clausesIsEmpty(){

// }

// void clausesHaveEmptyClause(){

// }

// bool dpll(const std::vector<Clause>& clauses){
    


//     //while loop1 unit-propagation
//     unitPropagation();
//     //while loop2 pure eliminate
//     pureEliminate(clauses);
//     // stop condition
//     // if1
//     if(clausesIsEmpty)
//         return true;
//     // if2
//     if(clausesHaveEmptyClause)
//         return false;
//     return false;

// }

int main() {
    std::ifstream file("./uf20-91/uf20-01.cnf"); // Open the file named "uf20-91"
    
    if (!file) { // Check if the file was successfully opened
        std::cerr << "Error: Could not open the file 'uf20-91'." << std::endl;
        return 1;
    }

    std::vector<Clause> clauses; // Vector to store all clauses
    std::string line;

    while (std::getline(file, line)) { // Read the file line by line
        if (line.empty() || line[0] == 'c' || line[0] == 'p') {
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

        clauses.push_back(clause); // Add the clause to the list
    }

    file.close(); // Close the file

    // Print all clauses for verification
    for (const Clause& clause : clauses) {
        clause.printClause();
    }

    

    // std::cout<<"[TEST]"<<clauses[0].literals[0]<<std::endl;

    // now we pick first element as our start point
    // dpll(clauses);


    return 0;
}