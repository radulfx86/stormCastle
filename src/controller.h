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
        Bounds *b = EntityManager::getInstance().getComponent<Bounds*>(target);
        MotionParameters_t *m = EntityManager::getInstance().getComponent<MotionParameters_t*>(target);
        printf("DBG controller for entity %d bounds at %p\n", target, &b);
        for ( Action &action : actions )
        {
            if ( action.TYPE == Action::MOTION )
            {
                if ( action.bParam )
                {
                    m->speed.x = action.v2iParam.x != 0 ? action.v2iParam.x : m->speed.x;
                    m->speed.y = action.v2iParam.y != 0 ? action.v2iParam.y : m->speed.y;
                }
                else
                {
                    m->speed.x = action.v2iParam.x != 0 ? 0 : m->speed.x;
                    m->speed.y = action.v2iParam.y != 0 ? 0 : m->speed.y;
                }
                //b->pos.x += action.v2iParam.x / 10.0;
                //b->pos.y += action.v2iParam.y / 10.0;
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