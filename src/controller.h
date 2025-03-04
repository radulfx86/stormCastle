#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "types.h"
#include "ecs.h"
#include "tools.h"

class Controller
{
    public:
    Controller(EntityID target) : target(target) {}

    virtual bool update(float delta_s)
    {
        (void) delta_s;
        Bounds *b = EntityManager::getInstance().getComponent<Bounds*>(target);
        MotionParameters_t *m = EntityManager::getInstance().getComponent<MotionParameters_t*>(target);
        CharacterState_t *st = EntityManager::getInstance().getComponent<CharacterState_t*>(target);
        printf("DBG controller for entity %d bounds at %p\n", target, &b);
        for ( Action &action : actions )
        {
            if ( action.TYPE == Action::MOTION )
            {
                if ( action.bParam )
                {
                    st->dir = Tools::dirFromVector(action.v2iParam);
                    m->speed.x = action.v2iParam.x != 0 ? action.v2iParam.x : m->speed.x;
                    m->speed.y = action.v2iParam.y != 0 ? action.v2iParam.y : m->speed.y;
                }
                else
                {
                    m->speed.x = action.v2iParam.x != 0 ? 0 : m->speed.x;
                    m->speed.y = action.v2iParam.y != 0 ? 0 : m->speed.y;
                }
            }
            else if ( action.TYPE == Action::INTERACT )
            {
                InteractionParameters_t *i = EntityManager::getInstance().getComponent<InteractionParameters_t*>(target);
                i->type = InteractionParameters_t::TRIGGER;
                i->direction = Tools::dirVector(st->dir);
                printf("new interaction - direction: %f %f\n", i->direction.x, i->direction.y);
                i->active = true;
            }
            else if ( action.TYPE == Action::SPECIAL )
            {
                InteractionParameters_t *i = EntityManager::getInstance().getComponent<InteractionParameters_t*>(target);
                i->type = InteractionParameters_t::RANGED;
                i->direction.x = action.v2iParam.x;
                i->direction.y = action.v2iParam.y;
                i->active = true;
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

    protected:
    const EntityID target;
    std::vector<Action> actions;
};

class DialogController : public Controller
{
public:
    DialogController(DialogTree *tree, EntityID source, EntityID target)
    : Controller(source), tree(tree), source(source), target(target) {}

    virtual bool update(float delta_s) override
    {
        printf("DialogController updated - has %d actions\n", actions.size());
        for (Action &action : actions)
        {
            // select menu entry
            if (action.TYPE == Action::MOTION)
            {
                if (action.v2iParam.y > 0)
                {
                    tree->selectedOption = (tree->selectedOption + 1) % tree->numOptions;
                }
                if (action.v2iParam.y < 0)
                {
                    tree->selectedOption = (tree->selectedOption - 1) % tree->numOptions;
                }
            }
            // trigger menu action
            else if (action.TYPE == Action::INTERACT)
            {
                if (tree->options[tree->selectedOption])
                {
                    tree = tree->options[tree->selectedOption];
                }
                else if (tree->triggers[tree->selectedOption])
                {
                    tree->triggers[tree->selectedOption](source, target);
                }
            }
            else if (action.TYPE == Action::SPECIAL)
            {
                printf("CLOSE DIALOG source: %d target: %d\n", source, target);
            }

        }
        actions.clear();
        return true;
    }

private:
    DialogTree *tree;
    EntityID source, target;
};

#endif // _CONTROLLER_H_