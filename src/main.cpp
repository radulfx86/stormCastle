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

EntityManager &s = EntityManager::getInstance();

/*** ECS STUFF HERE */



#include <iostream>

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
    }
    bool update(float delta_s)
    {
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

        /* collision */
        /// NOTE: somehow player entity is deleted ...
        for ( auto entity : s.getSystemEntities(collisionDetection) )
        {
            Bounds *bounds = s.getComponent<Bounds*>(entity);
                printf("entity %d pos at %f %f\n", entity, bounds->pos.x, bounds->pos.y);
            MotionParameters_t *motion = s.getComponent<MotionParameters_t*>(entity);
            Bounds tmpNextBounds = {
                Vec2{
                    bounds->pos.x + motion->speed.x * delta_s,
                    bounds->pos.y + motion->speed.y * delta_s },
                bounds->size
            };
            if ( data.intersects(tmpNextBounds) )
            {
                motion->speed.x = 0.f;
                motion->speed.y = 0.f;
                printf("entity %d collided with level - not moving!\n", entity);
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
    float idMat_01[] = {.1f,0,0,0,
                    0,.1f,0,0,
                    0,0,.1f,0,
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


EntityID player;

void move(const Vec2i &dir)
{
    Bounds &b = s.getComponent<Bounds>(player);
    printf("DBG player bounds at %p\n", &b);
    b.pos.x += dir.x/10.0;
    b.pos.y += dir.y/10.0;
    printf("move\n");
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

    player = s.newEntity("player");
    s.addComponent<Object2D*>(player, obj);
    Bounds *bounds = new Bounds;
    bounds->pos = Vec2{1,1};
    s.addComponent<Bounds*>(player, bounds);
    MotionParameters_t *motion = new MotionParameters_t;
    motion->speed = {0,0};
    s.addComponent<MotionParameters_t*>(player, motion);
    printf("DBG player bounds at %p\n", bounds);

    scene.controller = new Controller(player);

    InstancedObject2D *iobj = new InstancedObject2D;
    GLuint instancedProgram = createShader(loadText("shaders/simple.instanced.vs").c_str(), loadText("shaders/simple.instanced.fs").c_str());
    //createInstancedObject(*iobj, instancedProgram);
    createInstanceBackground(*iobj, instancedProgram);
    iobj->tex = loadTexture("assets/images/tiles.png",0);


    printf("player tex: %d level tex: %d\n", obj->tex, iobj->tex);

    EntityID background = s.newEntity("background");
    s.addComponent<Object2D*>(background,iobj);
    Bounds *bgbounds = new Bounds;
    bgbounds->pos = Vec2{1,1};
    s.addComponent<Bounds*>(background, bgbounds);
    MotionParameters_t *bgMotion = new MotionParameters_t;
    bgMotion->speed = {0,0};
    s.addComponent<MotionParameters_t*>(background, bgMotion);

    scene.currentLevel = new Level(LevelData::load("level.txt", Vec2i{-5,-5}), background);

    s.showAll();

    scene.tick = 0;

    startMainLoop(scene);

    //cleanScene(scene);

}
