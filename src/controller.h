#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "types.h"
#include "ecs.h"

class Controller
{
    public:
    Controller(EntityID target) : target(target) {}

    bool update(float delta_s)
    {
        (void) delta_s;
        Bounds &b = EntityManager::getInstance().getComponent<Bounds>(target);
        for ( Action &action : actions )
        {
            if ( action.TYPE == Action::MOTION )
            {
                b.pos.x += action.v2iParam.x / 10.0;
                b.pos.y += action.v2iParam.y / 10.0;
            }
        }
        actions.clear();
        return true;
    }

    void clearActions()
    {
        actions.clear();
    }

    void addAction(Action action)
    {
        actions.push_back(action);
    }

    private:
    const EntityID target;
    std::vector<Action> actions;
};

#endif // _CONTROLLER_H_