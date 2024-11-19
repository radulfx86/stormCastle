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

EntityManager &s = EntityManager::getInstance();

/*** ECS STUFF HERE */



#include <iostream>
#include <set>
#include <queue>

class LevelData
{
public:
    enum LevelTileType
    {
        WALL,
        DOOR,
        NUM_LEVEL_TILE_TYPES
    };
    struct LevelTile
    {
        LevelTileType type;
        int orientation;
    };
    int getTileIndex(Vec2i pos)
    {
        if ( data.find(pos) == data.end() )
        {
            return -1;
        }
        return data[pos].orientation;
    }

    LevelData() {}

    static LevelData load(std::string path, Vec2i offset)
    {
        LevelData level;
        int w = 0;
        int h = 0;
        try
        {
            std::ifstream istr(path, std::ios_base::in);
            int y = 0;
            for( std::string line; std::getline(istr, line); ++y)
            {
                printf("y: %2d ", y);
                for ( int x = 0; x < (int)line.length(); ++x )
                {
                    printf(">(%d,%d)<", x+offset.x,y+offset.y);
                    if ( x > w )
                    {
                        w = x;
                    }
                    switch (line[x])
                    {
                        case 'W' :
                            level.data[Vec2i{x+offset.x,y+offset.y}] = {WALL,0};
                            break;
                        case 'D' :
                            level.data[Vec2i{x+offset.x,y+offset.y}] = {DOOR,0};
                            break;
                        default:
                            break;
                    }
                }
                printf("\n");
                if ( y > h )
                {
                    h = y;
                }
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        // index in tile-set to be used
        // indices correspond to the sum of neighbor positions
        //  1		1/2		2  
        //  1/2		x		2/8
        //   4		4/8     8
        for ( std::pair<const Vec2i, LevelTile> &tile : level.data )
        {
            int x = tile.first.x;
            int y = tile.first.y;
            int cnt = 0;
            cnt += (level.data.find(Vec2i{x, y - 1}) != level.data.end())
                    && (level.data.find(Vec2i{x - 1, y - 1}) != level.data.end())
                    && (level.data.find(Vec2i{x - 1, y}) != level.data.end())
                 ? 4 : 0;
            cnt += (level.data.find(Vec2i{x, y - 1}) != level.data.end())
                    && (level.data.find(Vec2i{x + 1, y - 1}) != level.data.end())
                    && (level.data.find(Vec2i{x + 1, y}) != level.data.end())
                 ? 8 : 0;
            cnt += (level.data.find(Vec2i{x, y + 1}) != level.data.end())
                    && (level.data.find(Vec2i{x - 1, y + 1}) != level.data.end())
                    && (level.data.find(Vec2i{x - 1, y}) != level.data.end())
                 ? 1 : 0;
            cnt += (level.data.find(Vec2i{x, y + 1}) != level.data.end())
                    && (level.data.find(Vec2i{x + 1, y + 1}) != level.data.end())
                    && (level.data.find(Vec2i{x + 1, y}) != level.data.end())
                 ? 2 : 0;
            tile.second.orientation = cnt;
            printf("LEVEL %2d %2d -> %d\n", x, y, cnt);
        }
        printf("LEVEL size: %ld\n", level.data.size());
        
        return level;
    }
    bool intersects(const Bounds &b)
    {
        return data.find(Vec2i{(int)b.pos.x, (int)b.pos.y}) != data.end()
                || data.find(Vec2i{(int)(b.pos.x + b.size.x), (int)b.pos.y}) != data.end()
                || data.find(Vec2i{(int)(b.pos.x + b.size.x), (int)(b.pos.y + b.size.y)}) != data.end()
                || data.find(Vec2i{(int)b.pos.x, (int)(b.pos.y + b.size.y)}) != data.end();
    }
    std::vector<std::pair<Vec2i, LevelTile>> getTilesInBounds(const Bounds &b)
    {
        std::vector<std::pair<Vec2i, LevelTile>> tiles;
        Vec2i min = {(int)b.pos.x, (int)b.pos.y};
        Vec2i max = {(int)std::ceil(b.pos.x + b.size.x), (int)(std::ceil(b.pos.y + b.size.y))};
        for ( auto tile : data )
        {
            tiles.push_back(tile);
        }
        return tiles;
        for ( int x = min.x; x < max.x ; ++x )
        {
            for ( int y = min.y; y < max.y; ++y )
            {
                Vec2i tmp{x,y};
                if ( data.find(tmp) != data.end() )
                {
                    tiles.push_back(std::pair<Vec2i, LevelTile>(tmp, data[tmp]));
                }
            }
        }
        return tiles;
    }
    std::vector<Vec2i> getPathTo(Vec2i start, Vec2i end)
    {
        struct WeightedVec2i{
            Vec2i pos;
            int distanceSquared;
            std::vector<Vec2i> path;
        };
        std::vector<Vec2i> path;
        std::set<Vec2i> seen;
        auto cmp = [&](WeightedVec2i l, WeightedVec2i r) { return l.distanceSquared > r.distanceSquared; };
        std::priority_queue<WeightedVec2i, std::vector<WeightedVec2i>, decltype(cmp)> pq(cmp);

        auto getWeightedVec2i = [] (Vec2i pos, Vec2i target, std::vector<Vec2i> path) -> WeightedVec2i
        {
            path.push_back(pos);
            return WeightedVec2i{pos, (target.x - pos.x) * (target.x - pos.x) + (target.y - pos.y) * (target.y - pos.y), path};
        };

        pq.push(getWeightedVec2i(start, end, {}));
        do
        {
            WeightedVec2i el = pq.top();
            pq.pop();
            printf("test path of length %lu to pos %d %d (target %d %d) pq size: %lu\n", el.path.size(), el.pos.x, el.pos.y, end.x, end.y, pq.size());
            if (end == el.pos)
            {
                el.path.push_back(el.pos);
                path = el.path;
                break;
            }
            for (int x = -1; x < 2; ++x)
            {
                for (int y = -1; y < 2; ++y)
                {
                    if ( not (x == 0 || y == 0) )
                        continue;
                    Vec2i next{el.pos.x + x, el.pos.y + y};
                    if (data.find(next) != data.end())
                    {
                        continue;
                    }
                    std::vector<Vec2i> tmp = el.path;
                    pq.push(getWeightedVec2i(next, end, tmp));
                }
            }
        } while (not pq.empty());

        return path;
    }
private:
    std::map<Vec2i, LevelTile> data;
};

class Level
{
public:
    Level(LevelData data, EntityID background) : data(data), background(background), numTiles(100)
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


        std::vector<Vec2i> path = data.getPathTo(Vec2i{0,0}, Vec2i{3,3});
        for ( const Vec2i &el : path )
        {
            printf("path element %d %d\n", el.x, el.y);
        }
        //exit(3);
    }
    bool update(float delta_s)
    {
        s.updateSystem(actionHandling);
        /* level background */
        /* TODO:
        - get view-cone (optional)
        - for each tile (visible) in the level:
            - set background-tile-instance - pos & type (texture offset)
        */
        InstancedObject2D *bgObj = (InstancedObject2D*)s.getComponent<Object2D*>(background);
        Bounds cameraBounds{Vec2{-5,-5}, Vec2{10,10}};
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
            if ( interaction->active )
            {
                printf("interaction for entity %d of type %d dir(%f %f)\n", entity, interaction->type, interaction->direction.x, interaction->direction.y);
                Bounds *sourceBounds = s.getComponent<Bounds*>(entity);
                Bounds triggerBounds = *sourceBounds;
                triggerBounds.pos = triggerBounds.pos + interaction->direction;
                for ( auto triggerEntity : s.getSystemEntities(triggers) )
                {
                    if ( entity != triggerEntity )
                    {
                        Bounds *targetBounds = s.getComponent<Bounds*>(triggerEntity);
                        if ( Tools::doesIntersect(&triggerBounds, targetBounds) )
                        {
                            TriggerFunction triggerFunction = s.getComponent<TriggerFunction>(triggerEntity);
                            (triggerFunction)(triggerEntity, entity);
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
};

/**** NO MORE ECS STUFF HERE */
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
    obj.animation.frames = {
        {{0,0}, {w,1.f}, false,  {0,0}},
        {{1,0}, {w,1.f}, false,  {0,0}},
        {{2,0}, {w,1.f}, false,  {0,0}},
        {{3,0}, {w,1.f}, false,  {0,0}},
    };
}


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
            anim.frames.clear();// = {{{0,0}, {w,h}, false, {0,0}}};
            anim.currentFrame = 0;
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

    StateMachine *npcStateMachine = new StateMachine(npc);

    s.addComponent<Object2D*>(npc, npcObj);
    s.addComponent<Bounds*>(npc, npcBounds);
    s.addComponent<MotionParameters_t*>(npc, npcMotion);
    s.addComponent<StateMachine*>(npc, npcStateMachine);

    auto triggerFun = [](int target, int source) -> void {
        printf("NPC triggered target: %d, source: %d\n", target, source);
    };
    s.addComponent<TriggerFunction>(npc, (TriggerFunction) triggerFun);

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
EntityID addText()
{
    Text2D *obj = new Text2D;
    GLuint instancedProgram = createShader(loadText("shaders/simple.instanced.vs").c_str(), loadText("shaders/simple.instanced.fs").c_str());

    createObject(*obj, instancedProgram);
    obj->texOffset = 1;
    glUseProgram(obj->program);
    glUniform1i(glGetUniformLocation(obj->program, "tex"), 0);
    glUseProgram(0);
    obj->size = Vec2{1,1};
    obj->pos = Vec2{-2,0};
    obj->setCharacterSize(Vec2{1.0/36.0, 0.5});
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
    obj->setTextIndex(' ', c);
    obj->textureColumns = 36;

    obj->tex = loadTexture("assets/images/font4_8.png",0);
    EntityID text = s.newEntity("text");
    s.addComponent<Object2D*>(text,obj);
    Bounds *bgbounds = new Bounds;
    bgbounds->pos = Vec2{0,0};
    s.addComponent<Bounds*>(text, bgbounds);
    MotionParameters_t *bgMotion = new MotionParameters_t;
    bgMotion->speed = {0,0};
    s.addComponent<MotionParameters_t*>(text, bgMotion);
    printf("level tex: %d\n", obj->tex);



    obj->setText("hello World");

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
    EntityID npc = addNPC(obj->tex, program);
    printf("npc entity has id %d\n", npc);


    EntityID text = addText();
    (void) text;


    scene.currentLevel = new Level(LevelData::load("level.txt", Vec2i{-5,-5}), background);

    s.showAll();

    scene.tick = 0;

    startMainLoop(scene);

    //cleanScene(scene);

}
