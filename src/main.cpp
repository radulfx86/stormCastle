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

EntityManager &s = EntityManager::getInstance();

/*** ECS STUFF HERE */



#include <iostream>
class Level
{
public:
    Level()
    {
        //animations.init();
        Components drawingComponents;
        drawingComponents.set(s.getComponentID<Object2D *>());
        // drawingComponents.set(s.getComponentID<Bounds>());
        drawingSystem = s.addSystem(drawingComponents, "drawing");
    }
    bool update(float delta_s) { animations.update(delta_s); }
    bool draw(float delta_s)
    {
        int c = 0;
        for (auto entity : s.getSystemEntities(drawingSystem))
        {
            Bounds &b = s.getComponent<Bounds>(entity);
            Object2D t;
            printf("draw entity %d\n", entity);
            s.getComponent<Object2D *>(entity)->setPosition(s.getComponent<Bounds>(entity).pos);
            //s.getComponent<Object2D *>(entity)->update(delta_s);
            s.getComponent<Object2D *>(entity)->updateAnimation(delta_s);
            s.getComponent<Object2D *>(entity)->draw();
            // s.getComponent<Tile *>(entity)->printPointers();
            ++c;
        }
        //std::cerr << "elements drawn: " << c << "\n";
        printf("elements drawn: %d\n", c);
        return true;
    }

private:
    AnimationSystem animations;
    SystemID drawingSystem;
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
    for ( int x = -2; x < 2; ++x )
    {
        for ( int y = -2; y < 2; ++y )
        {
            obj.updateInstance(i++, true, Vec2{(float)x,(float)y}, Vec2{x,y}, Vec2{0.25,0.25});
            Animation anim;
            anim.frames.clear();// = {{{0,0}, {w,h}, false, {0,0}}};
            anim.currentFrame = 0;
            obj.instancePositions[Vec2i{x,y}] = i;
            //obj.animations.push_back(anim);
        }
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
    scene->currentLevel->draw(delta);

    scene->last = now;
    //printf("tick\n");
}


EntityID player;

void move(const Vec2i &dir)
{
    Bounds &b = s.getComponent<Bounds>(player);
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
    Bounds bounds;
    bounds.pos = Vec2{1,1};
    s.addComponent<Bounds>(player, bounds);

    scene.controller = new Controller(player);

    InstancedObject2D *iobj = new InstancedObject2D;
    GLuint instancedProgram = createShader(loadText("shaders/simple.instanced.vs").c_str(), loadText("shaders/simple.instanced.fs").c_str());
    //createInstancedObject(*iobj, instancedProgram);
    createInstanceBackground(*iobj, instancedProgram);
    iobj->tex = loadTexture("assets/images/tiles.png",0);


    printf("player tex: %d level tex: %d\n", obj->tex, iobj->tex);

    EntityID background = s.newEntity("background");
    s.addComponent<Object2D*>(background,iobj);
    Bounds bgbounds;
    bgbounds.pos = Vec2{1,1};
    s.addComponent<Bounds>(background, bgbounds);

    scene.currentLevel = new Level();

    s.showAll();

    startMainLoop(scene);

    //cleanScene(scene);

}