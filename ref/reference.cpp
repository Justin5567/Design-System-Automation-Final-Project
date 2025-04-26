#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

using namespace std;

// Literal: int (正整數代表正文字，負整數代表否定文字)
// Clause: set<int>
// CNF Formula: vector<Clause>
using Clause = set<int>;
using CNF = vector<Clause>;

// 選一個文字（這裡簡單選第一個找到的）
int chooseLiteral(const CNF& formula) {
    for (const auto& clause : formula) {
        for (int literal : clause) {
            return literal;
        }
    }
    return 0; // Should not reach here if formula not empty
}

// 單子句傳播（unit propagation）
bool unitPropagate(CNF& formula, int literal) {
    CNF newFormula;
    for (const auto& clause : formula) {
        if (clause.count(literal)) {
            continue; // 子句已滿足，跳過
        }
        if (clause.count(-literal)) {
            Clause newClause = clause;
            newClause.erase(-literal); // 移除與此相反的文字
            if (newClause.empty()) return false; // 空子句，衝突
            newFormula.push_back(newClause);
        } else {
            newFormula.push_back(clause); // 保留不相關子句
        }
    }
    formula = newFormula;
    return true;
}

// 純文字刪除
void eliminatePureLiterals(CNF& formula) {
    map<int, int> literalCount;
    for (const auto& clause : formula) {
        for (int literal : clause) {
            literalCount[literal]++;
        }
    }

    for (const auto& [literal, _] : literalCount) {
        if (literalCount.count(-literal) == 0) {
            // literal 是純的
            CNF newFormula;
            for (const auto& clause : formula) {
                if (!clause.count(literal)) {
                    newFormula.push_back(clause);
                }
            }
            formula = newFormula;
            // 只處理一個純文字，重啟掃描
            eliminatePureLiterals(formula);
            break;
        }
    }
}

// DPLL 主體遞迴函數
bool DPLL(CNF formula) {
    // 單子句傳播
    bool updated = true;
    while (updated) {
        updated = false;
        for (const auto& clause : formula) {
            if (clause.size() == 1) {
                int unit = *clause.begin();
                if (!unitPropagate(formula, unit)) return false;
                updated = true;
                break; // 每次只處理一個單子句
            }
        }
    }

    // 純文字刪除
    eliminatePureLiterals(formula);

    // 停止條件
    if (formula.empty()) return true;           // 全部子句都滿足
    for (const auto& clause : formula) {
        if (clause.empty()) return false;       // 有空子句 → 不可滿足
    }

    // 選擇一個變數進行分支
    int literal = chooseLiteral(formula);
    CNF copy1 = formula, copy2 = formula;
    if (unitPropagate(copy1, literal) && DPLL(copy1)) return true;
    if (unitPropagate(copy2, -literal) && DPLL(copy2)) return true;
    return false;
}

// 測試範例：Φ = (A ∨ B) ∧ (¬A ∨ C) ∧ (¬B ∨ ¬C) ∧ (A)
int main() {
    CNF formula = {
        {1, 2},    // A ∨ B
        {-1, 3},   // ¬A ∨ C
        {-2, -3},  // ¬B ∨ ¬C
        {1}        // A
    };

    if (DPLL(formula)) {
        cout << "SAT (可滿足)" << endl;
    } else {
        cout << "UNSAT (不可滿足)" << endl;
    }

    return 0;
}