#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "gl_headers.h"
#include "shaders.h"
#include "types.h"
#include "display.h"
#include "io.h"
#include "characters.h"
#include "ecs.h"
#include "systems.h"
#include "controller.h"
#include "tools.h"
#include "state_machine.h"
#include "object_factory.h"
#include "level.h"


/*** ECS STUFF HERE */


#include <sstream>

#include <iostream>
#include <set>
#include <queue>

class Level
{

private:
EntityManager &s = EntityManager::getInstance();
public:
    Level(LevelData data, EntityID background, EntityID player) : data(data), background(background), player(player), numTiles(100)
    {
        //animations.init();
        Components drawingComponents;
        drawingComponents.set(s.getComponentID<Object2D *>());
        // drawingComponents.set(s.getComponentID<Bounds>());
        drawingSystem = s.addSystem(drawingComponents, "drawing");

        Components collisionComponents;
        collisionComponents.set(s.getComponentID<Object2D *>());
        collisionComponents.set(s.getComponentID<Bounds *>());
        collisionDetection = s.addSystem(collisionComponents, "collision");

        Components actionComponents;
        actionComponents.set(s.getComponentID<InteractionParameters_t *>());
        actionHandling = s.addSystem(actionComponents, "actions");

        Components triggerComponents;
        triggerComponents.set(s.getComponentID<Bounds *>());
        triggerComponents.set(s.getComponentID<TriggerFunction>());
        triggers = s.addSystem(triggerComponents, "triggers");

        Components dialogComponents;
        dialogComponents.set(s.getComponentID<Bounds *>());
        dialogComponents.set(s.getComponentID<Dialog2D *>());
        dialogSystem = s.addSystem(dialogComponents, "dialogs");

        animations.init(true);


        pointer = s.newEntity("pointer");
        float purple[3]{1,0,1};
        s.addComponent<Text2D*>(pointer, ObjectFactory::getText({0,0},"",purple));


        camera.init(true);

        //exit(3);
    };
    bool update(float delta_s)
    {
        s.updateSystem(dialogSystem);

        s.updateSystem(actionHandling);

        /* camera */
        Bounds *playerBounds = (Bounds*)s.getComponent<Bounds*>(player);
        camera.move(playerBounds->pos);
        camera.update(delta_s);

        /* level background */
        /* TODO:
        - get view-cone (optional)
        - for each tile (visible) in the level:
            - set background-tile-instance - pos & type (texture offset)
        */
        InstancedObject2D *bgObj = (InstancedObject2D*)s.getComponent<Object2D*>(background);
        /// TODO get bounds from actual cam
        std::vector<std::pair<Vec2i, LevelData::LevelTile>> tiles = data.getTilesInBounds(camera.getViewCone());

        int idxTile = 0;
        for ( auto tile : tiles )
        {
            Vec2 texSize{(float)fmod(tile.second.orientation, 4.0),
                        tile.second.type * 4.0 + (float)(int)(tile.second.orientation / 4.0)};
            bgObj->updateInstanceTypePos(idxTile++, true,
                    Vec2{(float)tile.first.x, (float)tile.first.y},
                    texSize);
        }
        int newNumTiles = idxTile;
        // disable old tiles
        for ( ; idxTile < numTiles; ++idxTile )
        {
            bgObj->updateInstance(idxTile, false, Vec2{0,0}, Vec2{0,0}, Vec2{0,0});
        }
        printf("had %d tiles, now %d\n", numTiles, newNumTiles);
        numTiles = newNumTiles;

        /* interactions */
        for ( auto entity : s.getSystemEntities(actionHandling) )
        {
            InteractionParameters_t *interaction = s.getComponent<InteractionParameters_t*>(entity);
            CharacterState_t *st = s.getComponent<CharacterState_t*>(entity);
            if ( interaction->active )
            {
                if (interaction->type == InteractionParameters_t::RANGED)
                {
                   Text2D* pointerText = s.getComponent<Text2D*>(pointer);
                   pointerText->setPosition(interaction->direction); 
                   std::stringstream sstr;
                   sstr << "pos " << interaction->direction.x << " " << interaction->direction.y;
                   pointerText->setText(sstr.str());
                }
                else
                {
                    printf("interaction for entity %d of type %d dir(%f %f)\n", entity, interaction->type, interaction->direction.x, interaction->direction.y);
                    Bounds *sourceBounds = s.getComponent<Bounds *>(entity);
                    Bounds triggerBounds = *sourceBounds;
                    triggerBounds.pos = triggerBounds.pos + Tools::dirVector(st->dir);
                    for (auto triggerEntity : s.getSystemEntities(triggers))
                    {
                        if (entity != triggerEntity)
                        {
                            Bounds *targetBounds = s.getComponent<Bounds *>(triggerEntity);
                            if (Tools::doesIntersect(&triggerBounds, targetBounds))
                            {
                                printf("interaction bounds overlap : %d -> %d\n", entity, triggerEntity);
                                printf("interacted with NPC trigger function at %p\n", s.getComponent<TriggerFunction>(triggerEntity));
                                TriggerFunction triggerFunction = s.getComponent<TriggerFunction>(triggerEntity);
                                (triggerFunction)(triggerEntity, entity);
                            }
                        }
                    }
                }
            }
        }

        /* collision */
        for ( auto entity : s.getSystemEntities(collisionDetection) )
        {
            Bounds *bounds = s.getComponent<Bounds *>(entity);
            printf("entity %d pos at %f %f\n", entity, bounds->pos.x, bounds->pos.y);
            MotionParameters_t *motion = s.getComponent<MotionParameters_t *>(entity);
            Bounds tmpNextBounds = {
                Vec2{
                    bounds->pos.x + motion->speed.x * delta_s,
                    bounds->pos.y + motion->speed.y * delta_s},
                bounds->size};
            // entity <-> level intersection
            if (data.intersects(tmpNextBounds))
            {
                motion->speed.x = 0.f;
                motion->speed.y = 0.f;
                printf("entity %d collided with level - not moving!\n", entity);
            }
            // entity <-> entity intersection
            for ( auto other : s.getSystemEntities(collisionDetection) )
            {
                if (entity == other)
                {
                    continue;
                }
                Bounds *otherBounds = s.getComponent<Bounds *>(other);
                if (Tools::doesIntersect(&tmpNextBounds, otherBounds))
                {
                    motion->speed.x = 0.f;
                    motion->speed.y = 0.f;
                    printf("entity %d collided with entity %d - not moving!\n", other, entity);
                }
            }
            printf("entity %d pos at %f %f\n", entity, bounds->pos.x, bounds->pos.y);
            printf("entity %d m.s at %f %f d %f\n", entity, motion->speed.x, motion->speed.y, delta_s);
            bounds->pos.x += motion->speed.x * delta_s;
            bounds->pos.y += motion->speed.y * delta_s;
            printf("entity %d pos at %f %f\n", entity, bounds->pos.x, bounds->pos.y);
        }

        /* animations */
        animations.update(delta_s);

        return true;
    }
    bool draw(float delta_s)
    {
        /*
        std::vector<Vec2> path = data.getPathTo(Vec2i{-3,-3}, Vec2i{3,3});
        for ( const Vec2 &el : path )
        {
            printf("path element %f %f\n", el.x, el.y);
        }
        Path2D *p2d = 0;
        if ((Path2D*)s.getComponent<Object2D*>(dbg) )
        {
            p2d = (Path2D*)s.getComponent<Object2D*>(dbg);
        }
        else
        {
            p2d = new Path2D;
            s.addComponent<Object2D*>(dbg, p2d);
        }
        p2d->setPath(path);
        float yellow[] = {1,1,0};
        p2d->setColor(yellow);
        p2d->draw();
        */
        int c = 0;
        for (auto entity : s.getSystemEntities(drawingSystem))
        {
            Bounds *b = s.getComponent<Bounds *>(entity);
            Object2D *t = s.getComponent<Object2D *>(entity);
            printf("draw entity %d\n", entity);
            if ( t )
            {
                printf("DBG entity#%d bounds at %p\n", entity, b);
                printf("\tpos at %f %f\n", b->pos.x, b->pos.y);
                t->setPosition(b->pos);
                // s.getComponent<Object2D *>(entity)->update(delta_s);
                t->updateAnimation(delta_s);
                t->draw();
                printf("entity %d pos at %f %f\n", entity, b->pos.x, b->pos.y);
                // s.getComponent<Tile *>(entity)->printPointers();
            }
            else
            {
                printf("UNKOWN OBJECT FOR ENTITY %d\n", entity);
            }
            ++c;
        }
        /* UI */
        /* dialog */
        for ( auto entity : s.getSystemEntities(dialogSystem) )
        {
            Bounds *bounds = s.getComponent<Bounds *>(entity);
            Dialog2D *dialog = s.getComponent<Dialog2D *>(entity);
            dialog->draw();
        }
        //std::cerr << "elements drawn: " << c << "\n";
        printf("elements drawn: %d\n", c);
        return true;
    }

private:
    AnimationSystem animations;
    SystemID collisionDetection;
    SystemID drawingSystem;
    SystemID actionHandling;
    SystemID triggers;
    LevelData data;
    EntityID background;
    int numTiles;
    EntityID pointer;
    Camera camera;
    EntityID player;
    EntityID dbg;
    SystemID dialogSystem;
};

/**** NO MORE ECS STUFF HERE */

Scene2D scene;

void mainloop(void *userData)
{
    Scene2D *scene = (Scene2D*)userData;
    glClear(GL_COLOR_BUFFER_BIT);

    uint64_t now = getNow();
    if ( scene->last == 0)
    {
        scene->last = now;
    }
    float delta = (now - scene->last) / 1000.0;
    scene->controller->update(delta);
    scene->currentLevel->update(delta);
    //if ( scene->tick++ %10 == 0)
    {
        scene->currentLevel->draw(delta);
    }

    printf("FPS: %f\n", 1.0/delta);

    scene->last = now;
    //printf("tick\n");
}

void move(const Vec2i &dir)
{
    (void)dir;
    printf("DEPRECATED - DO NOT USE\n");
    exit(3);
}


int main()
{
    printf("main\n");
    initScene(scene);

    printf("setup ogl\n");

    glViewport(0,0,640,480);
    //glViewport(0,0,1280,960);
    glClearColor(0.2,0.2,0.2,1.0);

    printf("setup main loop\n");

    Object2D *obj = new Object2D;
    GLuint program = createShader(loadText("shaders/simple.vs").c_str(), loadText("shaders/simple.fs").c_str());
    ObjectFactory::createObject(*obj, program);
    obj->tex = loadTexture("assets/images/characters.png",0);

    EntityID player = ObjectFactory::initPlayer(obj);

    scene.controller = new Controller(player);

    printf("player tex: %d\n", obj->tex);

    EntityID background = ObjectFactory::initBackground();

#if 0
    /* trigger */
    auto triggerFun = [](int target, int source) -> void {
        printf("trigger enabled: target: %d, source: %d\n", target, source);
    };
    Bounds *triggerBounds = new Bounds;
    triggerBounds->size = {4,4};
    triggerBounds->pos = {-1,-1};
    EntityID trigger = s.newEntity("trigger");
    s.addComponent<Bounds*>(trigger, triggerBounds);
    s.addComponent<TriggerFunction>(trigger, (TriggerFunction) triggerFun);
    #endif

    /* NPC */
    EntityID npc = ObjectFactory::addNPC(obj->tex, createShader(loadText("shaders/simple.vs").c_str(), loadText("shaders/simple.fs").c_str()));
    //printf("npc entity has id %d\n", npc);


    EntityID text = ObjectFactory::addText(Vec2{0,2},"hello World");
    (void) text;

    scene.currentLevel = new Level(LevelData::load("level.txt", Vec2i{-5,-5}), background, player);

    EntityManager::getInstance().showAll();

    scene.tick = 0;

    startMainLoop(scene);

    //cleanScene(scene);

}
