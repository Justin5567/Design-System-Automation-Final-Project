# Design-System-Automation-Final-Project

In this repo, we implement the sat solver by using dpll. Furthermore, we also apply several technique in advanced version to speed up the program. We apply watch_literal to decrease the tracking literals; we apply backtrack to avoid redundant copy.

## How to build

naive version: g++ -o dpll dpll.cpp
advanced version: g++ -o dpll_advanced dpll_advanced

## How to run

./dpll YOURTESTCNF.cnf
./dpll_advanced YOURTESTCNF.cnf