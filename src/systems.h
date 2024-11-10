#ifndef _SYSTEMS_H_
#define _SYSTEMS_H_
#include "ecs.h"

typedef bool (*AnimationUpdater)(Object2D *tgt, MotionParameters_t &params);

class AnimationSystem : public System{
public:
    AnimationSystem() : System(EntityManager::getInstance().newSystem("animations"))
    {
        components.set(EntityManager::getInstance().getComponentID<MotionParameters_t>());
        components.set(EntityManager::getInstance().getComponentID<Object2D *>());
        components.set(EntityManager::getInstance().getComponentID<AnimationUpdater>());
    }

    virtual void update(float deltaTimeS) override
    {
        for ( EntityID eid : EntityManager::getInstance().getSystemEntities(id) )
        {
            AnimationUpdater updater = EntityManager::getInstance().getComponent<AnimationUpdater>(eid);
            MotionParameters_t &params = EntityManager::getInstance().getComponent<MotionParameters_t>(eid);
            Object2D *tgt = EntityManager::getInstance().getComponent<Object2D*>(eid);
            updater(tgt, params);
        }
    }
};
#endif // _SYSTEMS_H_