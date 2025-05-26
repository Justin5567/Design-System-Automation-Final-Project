#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <algorithm>
// #include "dpll.h"
#include <chrono>
#include <iomanip>
int iter = 0;

int last_pick;
// the data structure of Clause
struct Clause {
    std::vector<int> literals;
    int watch1;
    int watch2;

    Clause() : watch1(0), watch2(0) {}

    void addLiteral(int lit) {
        literals.push_back(lit);
    }

    void printClause() const {
        for (int lit : literals) {
            std::cout << lit << " ";
        }
        std::cout << "0\n";
    }
};

// assign positive and negative literal into index
int lit_to_idx(int literal){
    int literal_abs = std::abs(literal);
    if(literal<0){
        return 2*(literal_abs-1)+1;
    }
    else{
        return 2*(literal_abs-1);
    }
}

// initialize the watch literals
void initWatchLiterals( std::vector<Clause>& clauses, 
                        std::vector<int>& assignment ,
                        std::vector<std::vector<Clause*>>& watch_literals)
{
    for(Clause& clause: clauses){
        // if(clause.literals.size()==1){
        //     std::cout<<"!!!\n";
        // }
        if(clause.literals.size()==1){
            // std::cout<<"1\n";
            clause.watch1 = clause.literals[0];
            clause.watch2 = clause.literals[0];
            watch_literals[lit_to_idx(clause.watch1)].push_back(&clause);
        }   
        else{
            // std::cout<<"Size:"<<clause.literals.size()<<"\n";

            clause.watch1 = clause.literals[0];
            clause.watch2 = clause.literals[1];

            // std::cout<<"w1:"<<clause.watch1<<std::endl;
            // std::cout<<"w2:"<<clause.watch2<<std::endl;
            watch_literals[lit_to_idx(clause.watch1)].push_back(&clause);
            watch_literals[lit_to_idx(clause.watch2)].push_back(&clause);
        }
    }
}

// Pick the best literal base on most frequently unassign literal
int chooseLiteral(const std::vector<Clause>& clauses,
                  const std::vector<int>& assignment,
                  int max_count)
{
    for (const Clause& clause : clauses) {
        for (int lit : clause.literals) {
            int var = std::abs(lit);
            if (var > 0 && var <= max_count && assignment[var] == 0) {
                return lit;  
            }
        }
    }

    return 0; 
}
// reverse assignment by trail
void backtrack(int level,
               std::vector<int>& assignment,
               std::vector<int>& trail,
               std::vector<int>& trail_lim)
{
    if (trail_lim.empty() || level >= trail_lim.size()) return;

    int limit = trail_lim[level];
    while (trail.size() > limit) {
        int lit = trail.back();
        trail.pop_back();
        int var = std::abs(lit);
        if (var >= 1 && var < assignment.size())
            assignment[var] = 0;
    }

    trail_lim.resize(level);
}

// propagate the value to all clauses and check for three important cases
bool unitPropagate( //std::vector<Clause>& clauses, // Not directly used
                   std::vector<std::vector<Clause*>>& watch_literals,
                   std::vector<int>& assignment,
                   std::vector<int>& trail,
                   int max_var_id,
                   int& qhead)
{
    while (qhead < trail.size()) {
        int p = trail[qhead++];
        int false_literal = -p;
        int false_idx = lit_to_idx(false_literal);

        if (false_idx < 0 || false_idx >= watch_literals.size()) {
            continue;
        }

        std::vector<Clause*>& watched_by_false_literal = watch_literals[false_idx];
        std::vector<Clause*> next_watched_by_false_literal;
        bool conflict_detected_for_this_p = false;

        // Iterate over all clauses currently watched by false_literal
        for (Clause* clause : watched_by_false_literal) {
            int w1 = clause->watch1;
            int w2 = clause->watch2;
            int other_watch = (w1 == false_literal) ? w2 : w1;
            int var_other = std::abs(other_watch);

            if (var_other <= 0 || var_other > max_var_id) { // Should not happen with valid CNF
                next_watched_by_false_literal.push_back(clause); // Keep problematic clause watched for now
                continue;
            }
            int val_other = assignment[var_other];

            // Case 1: The other watched literal (other_watch) is TRUE
            if ((other_watch > 0 && val_other == 1) || (other_watch < 0 && val_other == -1)) {
                next_watched_by_false_literal.push_back(clause); // Keep false_literal watching
                continue;
            }

            // Case 2: Try to find a new literal to watch in this clause
            bool found_new_watch = false;
            for (int new_lit : clause->literals) {
                if (new_lit == false_literal || new_lit == other_watch) continue;

                int var_new = std::abs(new_lit);
                if (var_new <= 0 || var_new > max_var_id) continue; // Skip invalid literals
                int val_new = assignment[var_new];

                // If new_lit is not FALSE (i.e., it's TRUE or UNASSIGNED)
                if (!((new_lit > 0 && val_new == -1) || (new_lit < 0 && val_new == 1))) {
                    if (w1 == false_literal) clause->watch1 = new_lit;
                    else clause->watch2 = new_lit;
                    watch_literals[lit_to_idx(new_lit)].push_back(clause);
                    found_new_watch = true;
                    break;
                }
            }

            if (found_new_watch) {
                // Clause is now watched by new_lit and other_watch.
                // It's removed from false_literal's list by NOT adding to next_watched_by_false_literal.
                continue;
            }

            // Case 3: No new watch found. Clause is unit or conflicting.
            // false_literal must remain a watch. Add to next_watched_by_false_literal.
            next_watched_by_false_literal.push_back(clause);

            if (val_other == 0) { // Unit clause: other_watch is unassigned
                assignment[var_other] = (other_watch > 0) ? 1 : -1;
                trail.push_back(other_watch);
            } else { // Conflict: other_watch is also false (since it wasn't true in Case 1)
                conflict_detected_for_this_p = true;
                // We will return false after processing all clauses for this false_literal,
                // to ensure watch lists are fully updated.
            }
        }
        watched_by_false_literal = next_watched_by_false_literal; // Update the actual watch list

        if (conflict_detected_for_this_p) {
            // qhead = trail.size(); // This would stop further items on trail for this UP call,
                                  // but dpll will return false and backtrack anyway.
            return false; // A conflict was found related to literal p
        }
    }
    return true; // No conflicts detected
}


// the dpll function
bool dpll( std::vector<Clause>& clauses, // Pass by value or ensure it's not modified if used inside
           std::vector<int>& assignment,
           std::vector<std::vector<Clause*>>& watch_literals,
           int max_var_id,
           std::vector<int>& trail,
           std::vector<int>& trail_lim)
{
    int qhead = trail_lim.empty() ? 0 : trail_lim.back();

    // first initialize
    if (trail_lim.empty() && qhead == 0) { 
        for (const Clause& clause : clauses) {
            if (clause.literals.size() == 1) {
                int lit = clause.literals[0];
                int var = std::abs(lit);
                if (var > 0 && var <= max_var_id) {
                    if (assignment[var] == 0) {
                        assignment[var] = (lit > 0) ? 1 : -1;
                        trail.push_back(lit);
                    }
                }
            }
        }
    }

    if (!unitPropagate(watch_literals, assignment, trail, max_var_id, qhead)) {
        return false;
    }

    int pick_literal = chooseLiteral(clauses, assignment, max_var_id);
    // std::cout<<pick_literal<<std::endl;
    last_pick = pick_literal;
    if (pick_literal == 0) {
        return true; // SAT
    }

    int current_decision_level_index = trail_lim.size();

    // true
    trail_lim.push_back(trail.size()); 
    int var_picked = std::abs(pick_literal);
    assignment[var_picked] = (pick_literal > 0) ? 1 : -1;
    trail.push_back(pick_literal);
    if (dpll(clauses, assignment, watch_literals, max_var_id, trail, trail_lim)) {
        return true;
    }
    backtrack(current_decision_level_index, assignment, trail, trail_lim);

    // false
    trail_lim.push_back(trail.size()); 
    int neg_literal = -pick_literal;
    assignment[var_picked] = (neg_literal > 0) ? 1 : -1;
    trail.push_back(neg_literal);
    if (dpll(clauses, assignment, watch_literals, max_var_id, trail, trail_lim)) {
        return true;
    }
    backtrack(current_decision_level_index, assignment, trail, trail_lim);

    return false;
}

// main function
int main(int argc, char *argv[]) {
    using namespace std::chrono;

    std::string inFile = argv[1];
    std::ifstream file(inFile); // unsat test
    if (!file) { 
        std::cerr << "Error: Could not open the file." << std::endl;
        return 1;
    }

    std::vector<Clause> clauses; 
    std::string line;

    
    int max_count = 0;
    while (std::getline(file, line)) { 
        if (line.empty() || line[0] == 'c') {
            continue;
        }
        if (line[0] == 'p') {
            std::string p, cnf;
            int num_vars_expected, num_clauses_expected;
            std::stringstream ss(line);
            ss >> p >> cnf >> num_vars_expected >> num_clauses_expected;
            max_count = num_vars_expected;
            continue;
        }
        if (line.empty() || line[0] == 'c' || line[0] == 'p' || line[0] == '%' || line[0] == '0') {
            continue;
        }

        std::istringstream iss(line);
        Clause clause;
        int literal;

        while (iss >> literal) {
            if (literal == 0) {
                break; 
            }
            clause.addLiteral(literal);
        }
        clauses.push_back(clause); 
    }

    file.close(); 
    // std::cout<<"Max Count:"<<max_count<<std::endl;
    std::vector<int> assignment(max_count+1,0);
    std::vector<std::vector<Clause*>> watch_literals(2*max_count);
    std::vector<int> trail;
    std::vector<int> trail_lim;

    // for(Clause clause:clauses){
    //     clause.printClause();
    // }

    initWatchLiterals(clauses,assignment,watch_literals);
    last_pick = 0;
    using namespace std:: chrono;
    auto start = high_resolution_clock::now();
    bool ans = false;
    // trail_lim.push_back(0);
    ans = dpll(clauses,assignment, watch_literals,max_count,trail, trail_lim);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

   

    if(ans){
        std::cout<<"RESULT:SAT"<<std::endl;
        std::cout<<"ASSIGNMENT:";
        for (int i = 1; i < assignment.size(); i++) {
            std::cout << i << "=";
            if (assignment[i] == 1) std::cout << "1 ";
            else if (assignment[i] == -1) std::cout << "0 ";
            // else std::cout << "U "; // Unassigned
            // std::cout<<"\n";
        }
        std::cout<<"\n";
    }
    else{
        std::cout<<"RESULT:UNSAT"<<std::endl;
    }
    
    // std::cout << "\nExecution Time: " << duration.count() << " microseconds" << std::endl;

    return 0;

   
}
