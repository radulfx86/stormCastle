#ifndef _SYSTEMS_H_
#define _SYSTEMS_H_
#include "ecs.h"
#include <stdio.h>

typedef bool (*AnimationUpdater)(Object2D *tgt, MotionParameters_t *params);

class AnimationSystem : public System{
public:
    AnimationSystem() : System(EntityManager::getInstance().newSystem("animations"))
    {
        components.set(EntityManager::getInstance().getComponentID<MotionParameters_t *>());
        components.set(EntityManager::getInstance().getComponentID<Object2D *>());
        components.set(EntityManager::getInstance().getComponentID<AnimationUpdater>());
    }

    virtual void update(float deltaTimeS) override
    {
        for ( EntityID eid : EntityManager::getInstance().getSystemEntities(id) )
        {
            AnimationUpdater updater = EntityManager::getInstance().getComponent<AnimationUpdater>(eid);
            MotionParameters_t *params = EntityManager::getInstance().getComponent<MotionParameters_t*>(eid);
            Object2D *tgt = EntityManager::getInstance().getComponent<Object2D*>(eid);
            printf("updating AnimationsSystem for eid %d object at %p\n", eid, tgt);
            updater(tgt, params);
        }
    }
};

class Camera : public System
{
private:
    constexpr static Mat4 identity{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    Mat4 view;
    Mat4 proj;
    Vec2 pos;

public:
    Camera() : System(EntityManager::getInstance().newSystem("camera"))
    {
        memcpy(this->view, identity, sizeof(Mat4));
        memcpy(this->proj, identity, sizeof(Mat4));
        zoom(0.125);
    }
    void move(Vec2 pos)
    {
        this->pos = pos;
        printf("Camera: move to %f %f\n", pos.x, pos.y);
        view[12] = -pos.x;
        view[13] = -pos.y;
    }
    void zoom(float level)
    {
        proj[0] = level;
        proj[5] = level;
        proj[11] = level;
    }

    Bounds getViewCone()
    {
        Bounds viewBounds;
        viewBounds.size = Vec2{15,15};
        viewBounds.pos = Vec2{pos.x - viewBounds.size.x/2, pos.y - viewBounds.size.y/2};
        return viewBounds;
    }

    virtual void update(float deltaTimeS) override
    {
        printf("Camera: update\n");
        for (EntityID eid : EntityManager::getInstance().getSystemEntities(id))
        {
            printf("Camera: update for id %d\n", eid);
            Object2D *tgt = EntityManager::getInstance().getComponent<Object2D *>(eid);
            glUseProgram(tgt->program);
            glUniformMatrix4fv(glGetUniformLocation(tgt->program, "view"), 1, GL_FALSE, view);
            glUniformMatrix4fv(glGetUniformLocation(tgt->program, "projection"), 1, GL_FALSE, proj);
            glUseProgram(0);
        }
    }
};

#endif // _SYSTEMS_H_