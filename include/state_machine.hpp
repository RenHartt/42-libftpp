#pragma once

#include <map>
#include <functional>
#include <stdexcept>

template<typename TState>
class StateMachine {
private:
    bool isSet = false;
    TState currentState;
    std::map<TState, std::map<TState, std::function<void()>>> relationsMap;

public:
    void addState(const TState& state) {
        if (isSet == false) {
            currentState = state;
            isSet = true;
        }
    }

    void addAction(const TState& state, const std::function<void()>& lambda) {
        relationsMap[state][state] = lambda;
    }

    void addTransition(const TState& startState, const TState& finalState, const std::function<void()>& lambda) {
        relationsMap[startState][finalState] = lambda;
    }

    void transitionTo(const TState& state) {
        try {
            relationsMap.at(currentState).at(state)();
            currentState = state;
        } catch(const std::out_of_range& e) {
            throw std::invalid_argument("state not found");
        }
    }

    void update() {
        try {
            relationsMap.at(currentState).at(currentState)();
        } catch(const std::out_of_range& e) {
            throw std::invalid_argument("state not found");
        }
    }
};