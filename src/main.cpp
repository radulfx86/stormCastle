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

EntityManager &s = EntityManager::getInstance();

/*** ECS STUFF HERE */


#include <sstream>

#include <iostream>
#include <set>
#include <queue>

void printObjectInfo(Object2D &obj)
{
    printf("Object2D(vao: %d vertexBuffer: %d, program: %d)\n",
    obj.vao, obj.vertexBuffer, obj.program);
}



void createObject(Object2D &obj, GLuint program, float tileSize = 1.0, Vec2 spriteSize = {1,1}, Vec2i spritePos = {0,0})
{
    obj.program = program;
    glGenVertexArrays(1, &obj.vao);
    glBindVertexArray(obj.vao);

    glGenBuffers(1, &obj.vertexBuffer);
    float vertexData[] = {
        -tileSize/2.0f, tileSize/2.0f, spriteSize.x * spritePos.x, spriteSize.y * spritePos.y,
        -tileSize/2.0f, -tileSize/2.0f, spriteSize.x * spritePos.x, spriteSize.y * (1 + spritePos.y),
        tileSize/2.0f, tileSize/2.0f, spriteSize.x * (1 + spritePos.x), spriteSize.y * spritePos.y,
        tileSize/2.0f, -tileSize/2.0f, spriteSize.x * (1 + spritePos.x), spriteSize.y * (1 + spritePos.y)
       };
    glBindBuffer(GL_ARRAY_BUFFER, obj.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, 0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (GLvoid*)(2*sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    printf("created object: ");
    printObjectInfo(obj);

    obj.texOffset = 0;

    float idMat[] = {1,0,0,0,
                    0,1,0,0,
                    0,0,1,0,
                    0,0,0,1};
    float idMat_01[] = {.125f,0,0,0,
                    0,.125f,0,0,
                    0,0,.125f,0,
                    0,0,0,1};
    glUseProgram(obj.program);
    glUniform1i(glGetUniformLocation(obj.program, "tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(obj.program, "model"), 1, GL_FALSE, idMat);
    glUniformMatrix4fv(glGetUniformLocation(obj.program, "view"), 1,  GL_FALSE,idMat_01);
    glUniformMatrix4fv(glGetUniformLocation(obj.program, "projection"), 1,  GL_FALSE,idMat);
    glUseProgram(0);

    obj.animation.currentDelta = 0.0;
    obj.animation.currentFrame = 0;
    obj.animation.deltas = {0.2, 0.2, 0.2, 0.2 };
    float w = 1.0 / 12.0;
    obj.animation.currentDirection = Direction_t::DOWN;
    obj.animation.frames[DOWN] = {
        {{0,0}, {w,1.f}, false,  {0,0}},
        {{1,0}, {w,1.f}, false,  {0,0}},
        {{2,0}, {w,1.f}, false,  {0,0}},
        {{3,0}, {w,1.f}, false,  {0,0}},
    };
    obj.animation.frames[UP] = {
        {{4,0}, {w,1.f}, false,  {0,0}},
        {{5,0}, {w,1.f}, false,  {0,0}},
        {{6,0}, {w,1.f}, false,  {0,0}},
        {{7,0}, {w,1.f}, false,  {0,0}},
    };
    obj.animation.frames[LEFT] = {
        {{8,0}, {w,1.f}, true,  {0,0}},
        {{9,0}, {w,1.f}, true,  {0,0}},
        {{10,0}, {w,1.f}, true,  {0,0}},
        {{11,0}, {w,1.f}, true,  {0,0}},
    };
    obj.animation.frames[RIGHT] = {
        {{8,0}, {w,1.f}, false,  {0,0}},
        {{9,0}, {w,1.f}, false,  {0,0}},
        {{10,0}, {w,1.f}, false,  {0,0}},
        {{11,0}, {w,1.f}, false,  {0,0}},
    };
}
Text2D *getText(Vec2 pos, std::string content, float color[3])
{
    Text2D *obj = new Text2D;
    GLuint instancedProgram = createShader(loadText("shaders/simple.instanced.vs").c_str(), loadText("shaders/text.fs").c_str());

    createObject(*obj, instancedProgram);
    obj->texOffset = 1;
    glUseProgram(obj->program);
    glUniform1i(glGetUniformLocation(obj->program, "tex"), 0);
    glUseProgram(0);
    obj->size = Vec2{1,1};
    obj->pos = pos;
    obj->setCharacterSize(Vec2{1.0/36.0, 0.5}, Vec2{0.6,1});
    obj->numInstances = 0;
    int c = 0;
    for ( char i = 'a'; i <= 'z'; ++i, ++c )
    {
        obj->setTextIndex(i, c);
    }
    for ( char i = '0'; i <= '9'; ++i, ++c )
    {
        obj->setTextIndex(i, c);
    }
    for ( char i = 'A'; i <= 'Z'; ++i, ++c )
    {
        obj->setTextIndex(i, c);
    }
    obj->setTextIndex(' ', c++);
    obj->setTextIndex(',', c++);
    obj->setTextIndex('.', c++);
    obj->setTextIndex(';', c++);
    obj->setTextIndex(':', c++);
    obj->setTextIndex('!', c++);
    obj->setTextIndex('?', c++);
    obj->textureColumns = 36;

    obj->tex = loadTexture("assets/images/font8_12.png",0);

    obj->setText(content);
    obj->setColor(color);

    return obj;
}


class Level
{
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

        animations.init(true);


        pointer = s.newEntity("pointer");
        float purple[3]{1,0,1};
        s.addComponent<Text2D*>(pointer, getText({0,0},"",purple));


        camera.init(true);

        //exit(3);
    };
    bool update(float delta_s)
    {

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
        Bounds cameraBounds{Vec2{playerBounds->pos.x-2, playerBounds->pos.y-2}, Vec2{5,5}};
        std::vector<std::pair<Vec2i, LevelData::LevelTile>> tiles = data.getTilesInBounds(cameraBounds);

        int idxTile = 0;
        for ( auto tile : tiles )
        {
            Vec2 texSize{(float)fmod(tile.second.orientation, 4.0),
                        (float)(int)(tile.second.orientation / 4.0)};
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
        std::vector<Vec2> path = data.getPathTo(Vec2i{-3,-3}, Vec2i{3,3});
        for ( const Vec2 &el : path )
        {
            printf("path element %f %f\n", el.x, el.y);
        }
        float yellow[] = {1,1,0};
        Path2D p(path,yellow);
        p.draw();
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
};

/**** NO MORE ECS STUFF HERE */

void createInstanceBackground(InstancedObject2D &obj, GLuint program)
{
    createObject(obj, program);
    obj.texOffset = 1;
    glUseProgram(obj.program);
    glUniform1i(glGetUniformLocation(obj.program, "tex"), 0);
    glUseProgram(0);
    obj.size = Vec2{1,1};
    obj.pos = Vec2{0,0};
    int i = 0;
    float w = 0.25;
    float h = 0.25;
    for ( int x = -5; x < 5; ++x )
    {
        for ( int y = -7; y < 7; ++y )
        {
            obj.updateInstance(i++, true, Vec2{(float)x,(float)y}, Vec2{(float)x,(float)y}, Vec2{w, h});
            Animation anim;
            //anim.frames.clear();// = {{{0,0}, {w,h}, false, {0,0}}};
            anim.currentFrame = 0;
            anim.currentDirection = DOWN;
            //obj.instancePositions[Vec2i{x,y}] = i;
            //obj.animations.push_back(anim);
            if ( i >= 100 )
                break;
        }
            if ( i >= 100 )
                break;
    }
    obj.numInstances = i;
}

void createInstancedObject(InstancedObject2D &obj, GLuint program)
{
    createObject(obj, program);
    obj.size = Vec2{1,1};
    obj.pos = Vec2{0,0};
    int i = 0;
    for ( int x = -2; x < 2; ++x )
    {
        for ( int y = -2; y < 2; ++y )
        {
            obj.updateInstance(i++, true, Vec2{(float)x,(float)y}, Vec2{0,0}, Vec2{0.125,1});
            Animation anim;
            anim = obj.animation;
            anim.currentFrame = i % obj.animation.frames.size();
            obj.instancePositions[Vec2i{x,y}] = i;
            obj.animations.push_back(anim);
        }
    }
    obj.numInstances = i;
}


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

    scene->last = now;
    //printf("tick\n");
}

void move(const Vec2i &dir)
{
    (void)dir;
    printf("DEPRECATED - DO NOT USE\n");
    exit(3);
}


EntityID addNPC(GLuint tex, GLuint program)
{
    EntityID npc = s.newEntity("npc");
    Bounds *npcBounds = new Bounds;
    npcBounds->size = {1,1};
    npcBounds->pos = {2,2};
    Object2D *npcObj = new Object2D;
    createObject(*npcObj, program);
    npcObj->tex = tex;
    MotionParameters_t *npcMotion = new MotionParameters_t;
    npcMotion->speed = {0,0};

    CharacterState_t *npcState = new CharacterState_t;
    npcState->dir = DOWN;
    npcState->placeholder = 0;

    StateMachine *npcStateMachine = new StateMachine(npc);

    s.addComponent<Object2D*>(npc, npcObj);
    s.addComponent<Bounds*>(npc, npcBounds);
    s.addComponent<MotionParameters_t*>(npc, npcMotion);
    s.addComponent<StateMachine*>(npc, npcStateMachine);
    s.addComponent<CharacterState_t*>(npc, npcState);

    auto triggerFun = [](int target, int source) -> void {
        printf("NPC triggered target: %d, source: %d\n", target, source);
        //Dialog2D *diag = new Dialog2D;
        Text2D *text = s.getComponent<Text2D*>(source);
        Object2D *obj = s.getComponent<Object2D*>(source);
        if ( text == 0 )
        {
            float blue[3]{0,0,1};
            text = getText(obj->pos,"blubb", blue);
        }
        text->setText("blobb");
    };
    s.addComponent<TriggerFunction>(npc, (TriggerFunction) triggerFun);
    printf("NPC trigger function at %p\n", s.getComponent<TriggerFunction>(npc));
   
    s.addComponent<AnimationUpdater>(npc, [](Object2D *t, MotionParameters_t *p) -> bool {
        return false;
        Direction_t animDir = DOWN;
        bool update = false;
        if ( p->speed.x >= .5 )
        {
            animDir = RIGHT;
            update = true;
        }
        else if (p->speed.x <= -.5 )
        {
            animDir = LEFT;
            update = true;
        }
        else if ( p->speed.y >= 0.5 )
        {
            animDir = UP;
            update = true;
        }
        else if ( p->speed.y <= -0.5 )
        {
            animDir = DOWN;
            update = true;
        }
        if ( update )
        {
            t->setAnimation(animDir);
        }
        return false; });

    return npc;
}

EntityID initBackground()
{
    InstancedObject2D *iobj = new InstancedObject2D;
    GLuint instancedProgram = createShader(loadText("shaders/simple.instanced.vs").c_str(), loadText("shaders/simple.instanced.fs").c_str());
    //createInstancedObject(*iobj, instancedProgram);
    createInstanceBackground(*iobj, instancedProgram);
    iobj->tex = loadTexture("assets/images/tiles.png",0);
    EntityID background = s.newEntity("background");
    s.addComponent<Object2D*>(background,iobj);
    Bounds *bgbounds = new Bounds;
    bgbounds->pos = Vec2{0,0};
    s.addComponent<Bounds*>(background, bgbounds);
    MotionParameters_t *bgMotion = new MotionParameters_t;
    bgMotion->speed = {0,0};
    s.addComponent<MotionParameters_t*>(background, bgMotion);
    printf("level tex: %d\n", iobj->tex);

    return background;
}


EntityID addText(Vec2 pos, std::string content)
{
    float red[3]{1,0,1};
    Text2D *obj = getText(pos, content, red);
   
    EntityID text = s.newEntity("text");
    s.addComponent<Object2D*>(text,obj);
    Bounds *bgbounds = new Bounds;
    bgbounds->pos = Vec2{0,0};
    s.addComponent<Bounds*>(text, bgbounds);
    MotionParameters_t *bgMotion = new MotionParameters_t;
    bgMotion->speed = {0,0};
    s.addComponent<MotionParameters_t*>(text, bgMotion);
    printf("level tex: %d\n", obj->tex);

    return text;

}

EntityID initPlayer(Object2D *obj)
{
    EntityID player = s.newEntity("player");
    s.addComponent<Object2D*>(player, obj);
    Bounds *bounds = new Bounds;
    bounds->pos = Vec2{1,1};
    bounds->size = Vec2{1,1};
    s.addComponent<Bounds*>(player, bounds);
    MotionParameters_t *motion = new MotionParameters_t;
    motion->speed = {0,0};
    s.addComponent<MotionParameters_t*>(player, motion);
    printf("DBG player bounds at %p\n", bounds);
    InteractionParameters_t *interaction = new InteractionParameters_t;
    interaction->active = false;
    s.addComponent<InteractionParameters_t*>(player, interaction);

    CharacterState_t *npcState = new CharacterState_t;
    npcState->dir = DOWN;
    npcState->placeholder = 0;

    s.addComponent<AnimationUpdater>(player, [](Object2D *t, MotionParameters_t *p) -> bool {
        Direction_t animDir = DOWN;
        bool update = false;
        if ( p->speed.x >= .5 )
        {
            animDir = RIGHT;
            update = true;
        }
        else if (p->speed.x <= -.5 )
        {
            animDir = LEFT;
            update = true;
        }
        else if ( p->speed.y >= 0.5 )
        {
            animDir = UP;
            update = true;
        }
        else if ( p->speed.y <= -0.5 )
        {
            animDir = DOWN;
            update = true;
        }
        if ( update )
        {
            t->setAnimation(animDir);
        }
        return false; });
        return player;
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
    createObject(*obj, program);
    obj->tex = loadTexture("assets/images/characters.png",0);

    EntityID player = initPlayer(obj);

    scene.controller = new Controller(player);

    printf("player tex: %d\n", obj->tex);

    EntityID background = initBackground();

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
    EntityID npc = addNPC(obj->tex, createShader(loadText("shaders/simple.vs").c_str(), loadText("shaders/simple.fs").c_str()));
    //printf("npc entity has id %d\n", npc);


    EntityID text = addText(Vec2{0,2},"hello World");
    (void) text;

    scene.currentLevel = new Level(LevelData::load("level.txt", Vec2i{-5,-5}), background, player);

    s.showAll();

    scene.tick = 0;

    startMainLoop(scene);

    //cleanScene(scene);

}
