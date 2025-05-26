#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include "dpll.h"
#include <chrono>
#include <iomanip>
int iter = 0;

// check if there only consist positive or negative literal in all clauses
void eliminatePureLiterals(std::vector<Clause>& clauses, std::unordered_map<int, bool>& assignment) {
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

    for (const auto& [literal, _] : isPure) {
        int var = std::abs(literal);
        if (assignment.count(var) == 0) { 
            assignment[var] = (literal > 0);
        }
    }

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
}

// select first literal from first clauses
int chooseLiteral(const std::vector<Clause> & clauses){
    for(const auto& clause : clauses){
        return clause.literals.begin()->first;
    }
    return 0;
}

// count and sort literals from largest number of occurences to smallest
int DLIS(const std::vector<Clause>& clauses, const std::unordered_map<int, bool>& assignment) {
    std::unordered_map<int, int> literalCount;

    for (const auto& clause : clauses) {
        for (const auto& literal : clause.literals) {
            int var = std::abs(literal.first);
            if (assignment.count(var) == 0) {
                literalCount[literal.first]++;
            }
        }
    }
    if (literalCount.empty()) return 0;
    auto best = std::max_element(
        literalCount.begin(),
        literalCount.end(),
        [](const std::pair<const int, int>& a, const std::pair<const int, int>& b) {
            return a.second < b.second;
        }
    );
    // std::cout << "DLIS pick: Literal " << best->first << " (count = " << best->second << ")\n";
    return best->first;
}

// set the picking literal as true (false if is negative)
// simplify the whole clauses
bool unitPropagate(std::vector<Clause>& clauses, int literal, std::unordered_map<int, bool>& assignment){
    assignment[std::abs(literal)] = (literal > 0); // record the result
    std::vector<Clause> newClauses;
    for(const auto& clause: clauses){
        if(clause.literals.count(literal)) continue; // skip select literal
        if(clause.literals.count(-literal)){
            Clause newClause = clause;
            newClause.literals.erase(-literal);
            if(newClause.literals.empty()) return false; // if removing literal prime leading to some clause empty -> false
            newClauses.push_back(newClause);
        }
        else{
            newClauses.push_back(clause); // others remain
        }
    }
    clauses = newClauses;
    return true;
}

bool dpll(std::vector<Clause>& clauses, std::unordered_map<int, bool>& assignment){
    iter ++;
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
    
    eliminatePureLiterals(clauses,assignment);
    int literal;
    // literal = DLIS(clauses,assignment);
    literal = chooseLiteral(clauses);

    
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
        assignment = std::move(assignment_copy1);
        return true;
    }
    if(unitPropagate(copy2, -literal, assignment_copy2) && dpll(copy2, assignment_copy2)) {
        assignment = std::move(assignment_copy2);
        return true;
    }
    
    return false;

}


// chooseLiteral for watch literal version this version select base on current assignment and freq
int chooseLiteral_watchLiteral(const std::vector<Clause>& clauses, const std::unordered_map<int, bool>& assignment) {
    std::unordered_map<int, int> freq;
    // get the frequency count first
    for (const Clause& clause : clauses) {
        for (const auto& [lit, _] : clause.literals) {
            int var = std::abs(lit);
            if (assignment.count(var) == 0) {
                freq[lit]++;
            }
        }
    }

    if (freq.empty()) return 0;

    // Return the literal with max frequency
    return std::max_element(freq.begin(), freq.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        })->first;
}

bool unitPropagate_watchLiterals(std::vector<Clause>& clauses,
                                  int literal,
                                  std::unordered_map<int, bool>& assignment,
                                  std::unordered_map<int, std::vector<Clause*>>& watch_literals) {
    int var = std::abs(literal);
    bool value = (literal > 0);
    if (assignment.count(var)) {
        if (assignment[var] != value) return false;  // Conflict
        return true;
    }
    assignment[var] = value;

    int false_lit = -literal;
    auto watched_clauses = std::move(watch_literals[false_lit]);
    watch_literals[false_lit].clear();

    for (Clause* clause : watched_clauses) {
        int& w1 = clause->watch1;
        int& w2 = clause->watch2;

        int other = (w1 == false_lit) ? w2 : w1;
        bool moved = false;

        // Try to find a new literal to watch
        for (const auto& [lit, _] : clause->literals) {
            if (lit != false_lit && lit != other) {
                int abs_lit = std::abs(lit);
                if (!assignment.count(abs_lit) ||
                    (lit > 0 && assignment[abs_lit]) ||
                    (lit < 0 && !assignment[abs_lit])) {
                    if (w1 == false_lit) w1 = lit;
                    else w2 = lit;
                    watch_literals[lit].push_back(clause);
                    moved = true;
                    break;
                }
            }
        }

        if (!moved) {
            int abs_other = std::abs(other);
            if (!assignment.count(abs_other)) {
                if (!unitPropagate_watchLiterals(clauses, other, assignment, watch_literals)) return false;
            } else if ((other > 0 && !assignment[abs_other]) || (other < 0 && assignment[abs_other])) {
                return false;  // Conflict: both watched literals are false
            }
            watch_literals[other].push_back(clause);  // Still valid
        }
    }

    return true;
}

bool dpll_watchLiterals(std::vector<Clause>& clauses, std::unordered_map<int, bool>& assignment, std::unordered_map<int, std::vector<Clause*>>& watch_literals){
    
    bool changed = true;
    while (changed) {
        changed = false;
        for (Clause& clause : clauses) {
            if (clause.watch1 == clause.watch2) {
                int lit = clause.watch1;
                int var = std::abs(lit);
                if (!assignment.count(var)) {
                    if (!unitPropagate_watchLiterals(clauses, lit, assignment, watch_literals)) {
                        return false;
                    }
                    changed = true;
                    break;
                }
            }
        }
    }

    bool allClausesSatisfied = true;
    for (const Clause& clause : clauses) {
        bool satisfied = false;
        for (const auto& [lit, _] : clause.literals) {
            int var = std::abs(lit);
            if (assignment.count(var) &&
                ((lit > 0 && assignment[var]) || (lit < 0 && !assignment[var]))) {
                satisfied = true;
                break;
            }
        }
        if (!satisfied) {
            allClausesSatisfied = false;
            break;
        }
    }
    if (allClausesSatisfied) return true;


    int literal = chooseLiteral_watchLiteral(clauses,assignment);
    // std::cout<<"pirck lit: "<<literal<<std::endl;
    if(literal==0) return false; // no more literal can be picked
    
    std::vector<Clause> copy1 = clauses;
    std::vector<Clause> copy2 = clauses;
    std::unordered_map<int, bool> assignment_copy1 = assignment;
    std::unordered_map<int, bool> assignment_copy2 = assignment;
    std::unordered_map<int, std::vector<Clause*>> watch_copy1;
    std::unordered_map<int, std::vector<Clause*>> watch_copy2;

    for(Clause& clause: copy1){
        if(clause.watch1==clause.watch2){
            watch_copy1[clause.watch1].push_back(&clause);
        }
        else{
            watch_copy1[clause.watch1].push_back(&clause);
            watch_copy1[clause.watch2].push_back(&clause);
        }
    }

    for(Clause& clause: copy2){
        if(clause.watch1==clause.watch2){
            watch_copy2[clause.watch1].push_back(&clause);
        }
        else{
            watch_copy2[clause.watch1].push_back(&clause);
            watch_copy2[clause.watch2].push_back(&clause);
        }
    }
    // std::cout<<"enter left"<<std::endl;
    if(unitPropagate_watchLiterals(copy1, literal, assignment_copy1, watch_copy1) && dpll_watchLiterals(copy1, assignment_copy1,watch_copy1)) {
        assignment = std::move(assignment_copy1);
        return true;
    }
    // std::cout<<"exit left"<<std::endl;
    // std::cout<<"enter right"<<std::endl;
    if(unitPropagate_watchLiterals(copy2, -literal, assignment_copy2, watch_copy2) && dpll_watchLiterals(copy2, assignment_copy2, watch_copy2)) {
        assignment = std::move(assignment_copy2);
        
        return true;
    }
    // std::cout<<"exit right"<<std::endl;
    
    return false;

}


int main() {
    using namespace std::chrono;

    for (int i = 1; i <= 1000; ++i) {
        std::ostringstream filename_stream;
        if(i<10){
            filename_stream << "./uuf50-218/UUF50.218.1000/uuf50-" << std::setw(2) << std::setfill('0') << i << ".cnf";
        }
        else if(i<100){
            filename_stream << "./uuf50-218/UUF50.218.1000/uuf50-" << std::setw(3) << std::setfill('0') << i << ".cnf";
        }
        else{
            filename_stream << "./uuf50-218/UUF50.218.1000/uuf50-" << std::setw(4) << std::setfill('0') << i << ".cnf";
        }
        
        
        std::string filename = filename_stream.str();

        std::ifstream file(filename);
        if (!file) { 
            std::cerr << "Error: Could not open file '" << filename << "'." << std::endl;
            continue; // skip to next file
        }

        std::vector<Clause> clauses; 
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == 'c' || line[0] == 'p' || line[0] == '%' || line[0] == '0') {
                continue;
            }

            std::istringstream iss(line);
            Clause clause;
            int literal;
            while (iss >> literal) {
                if (literal == 0) break;
                clause.addLiteral(literal);
            }
            clauses.push_back(clause); 
        }

        file.close();

        std::unordered_map<int, bool> assignment;
        std::unordered_map<int, std::vector<Clause*>> watch_literals;
        
        for(auto& clause: clauses){
            auto it = clause.literals.begin();
            if(clause.literals.size() >= 1){
                clause.watch1 = it->first;
                watch_literals[it->first].push_back(&clause);
                ++it;
            }
            if(it != clause.literals.end()){
                clause.watch2 = it->first;
                watch_literals[it->first].push_back(&clause);
            }
        }

        auto start = high_resolution_clock::now();
        bool ans = dpll_watchLiterals(clauses, assignment, watch_literals);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);

        std::vector<std::pair<int, bool>> sorted_assign(assignment.begin(), assignment.end());
        std::sort(sorted_assign.begin(), sorted_assign.end());

        std::cout << "File: " << filename << " => ";
        if (ans) {
            std::cout << "SAT | ";
            for (const auto& [var, value] : sorted_assign) {
                std::cout << var << "=" << value << " ";
            }
        } else {
            std::cout << "UNSAT";
        }
        std::cout << "| Time: " << duration.count() << "us" << std::endl;
    }

    return 0;
}