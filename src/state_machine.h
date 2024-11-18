#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_
#include "types.h"

class StateMachine
{
    enum States
    {
        IDLE = 0,
        BUSY,
        TALKING,
        MOVING,
        ATTACKING_MELEE,
        ATTACKING_RANGED,
        HIT,
        DYING,
        DEAD,
        NUM_STATES
    };
    struct Condition
    {
        std::bitset<States::NUM_STATES> transitionExists;
        States state;
        bool (*check)(EntityID entity);
    };

public:
    void propagate()
    {
        for (const Condition &condition : conditions)
        {
            if (not condition.transitionExists[currentState])
            {
                continue;
            }
            if (condition.check(entity))
            {
                currentState = condition.state;
                break;
            }
        }
    }

    StateMachine(EntityID entity,
                States initialState = States::IDLE,
                std::bitset<States::NUM_STATES> statesEnabled = {States::IDLE},
                std::vector<Condition> conditions = {})
                : entity(entity),
                  statesEnabled(statesEnabled),
                  currentState(initialState),
                  conditions(conditions)
                {}
    void enableState(States state)
    {
        statesEnabled[state] = true;
    }
    void disableState(States state)
    {
        statesEnabled[state] = false;
    }
    void addState(States state, Condition condition)
    {
        conditions.push_back(condition);
        enableState(state);
    }

private:
    const EntityID entity;
    std::bitset<States::NUM_STATES> statesEnabled;
    States currentState;
    std::vector<Condition> conditions;
};

#endif // _STATE_MACHINE_H_