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

EntityManager &s = EntityManager::getInstance();

/*** ECS STUFF HERE */


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
            s.getComponent<Object2D *>(entity)->setPosition(s.getComponent<Bounds>(entity).pos);
            s.getComponent<Object2D *>(entity)->update(delta_s);
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

void Object2D::draw()
{
    //printf("drawObject: ");
    //printObjectInfo(obj);
    glUseProgram(this->program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->tex);

    glBindVertexArray(this->vao);

    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glBindVertexArray(0);

    glUseProgram(0);
}

void Object2D::setPosition(Vec2 pos)
{
    printf("move to %2.2f %2.2f\n", pos.x, pos.y);
    float idMat[] = {1,0,0,0,
                    0,1,0,0,
                    0,0,1,0,
                    pos.x,pos.y,0,1};
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, idMat);
    glUseProgram(0);
}

GLuint loadTexture(const char *path, GLuint offset)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

#ifdef __EMSCRIPTEN__
    int w, h;
    char *data = emscripten_get_preloaded_image_data(path, &w, &h);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    printf("loaded image of size %d x %d\n", w, h);
    free(data);
#else
    SDL_Surface *surface = IMG_Load(path);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    SDL_FreeSurface(surface);

#endif

    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_NEAREST);


    return tex;
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


void Object2D::update(float delta_s)
{
    bool updated = false;
    if ( this->animation.frames.size() > 1 )
    {
        this->animation.currentDelta += delta_s;

        while ( this->animation.deltas[this->animation.currentFrame] < this->animation.currentDelta )
        {
            this->animation.currentDelta -= this->animation.deltas[this->animation.currentFrame];
            this->animation.currentFrame = (this->animation.currentFrame + 1) % this->animation.deltas.size();
            updated = true;
        }
    }
    if ( updated )
    {
        printf("updated to frame %d at %0.2fx%0.2f size %0.2fx%0.2f\n",
               this->animation.currentFrame,
               this->animation.frames[this->animation.currentFrame].texPos[0],
               this->animation.frames[this->animation.currentFrame].texPos[1],
               this->animation.frames[this->animation.currentFrame].texSize[0],
               this->animation.frames[this->animation.currentFrame].texSize[1]);
        glUseProgram(this->program);
        GLuint enabledUniform = glGetUniformLocation(this->program, "texInfo.flip");
        glUniform1i(enabledUniform, this->animation.frames[this->animation.currentFrame].flip);
        GLuint posUniform = glGetUniformLocation(this->program, "texInfo.texOrigin");
        glUniform2fv(posUniform, 1, this->animation.frames[this->animation.currentFrame].texOrigin);
        GLuint texPosUniform = glGetUniformLocation(this->program, "texInfo.texPos");
        glUniform2fv(texPosUniform, 1, this->animation.frames[this->animation.currentFrame].texPos);
        GLuint texSizeUniform = glGetUniformLocation(this->program, "texInfo.texSize");
        glUniform2fv(texSizeUniform, 1, this->animation.frames[this->animation.currentFrame].texSize);
        glUseProgram(0);
    }
}

void InstancedObject2D::update(float delta_s)
{
    int idx = 0;
    for ( Animation &anim : this->animations )
    {
        bool updated = false;
        if (anim.frames.size() > 1)
        {
            anim.currentDelta += delta_s;

            while (anim.deltas[anim.currentFrame] < anim.currentDelta)
            {
                anim.currentDelta -= anim.deltas[anim.currentFrame];
                anim.currentFrame = (anim.currentFrame + 1) % anim.deltas.size();
                updated = true;
            }
        }
        if (updated)
        {
            printf("updated %d to frame %d at %0.2fx%0.2f size %0.2fx%0.2f\n",
                    idx,
                   anim.currentFrame,
                   anim.frames[anim.currentFrame].texPos[0],
                   anim.frames[anim.currentFrame].texPos[1],
                   anim.frames[anim.currentFrame].texSize[0],
                   anim.frames[anim.currentFrame].texSize[1]);
            glUseProgram(this->program);
            char uniformName[50];
            sprintf(uniformName,"texInfo[%i].flip", idx);
            GLuint enabledUniform = glGetUniformLocation(this->program, uniformName);
            glUniform1i(enabledUniform, anim.frames[anim.currentFrame].flip);
            sprintf(uniformName,"texInfo[%i].texOrigin", idx);
            GLuint posUniform = glGetUniformLocation(this->program, uniformName);
            glUniform2fv(posUniform, 1, anim.frames[anim.currentFrame].texOrigin);
            sprintf(uniformName,"texInfo[%i].texPos", idx);
            GLuint texPosUniform = glGetUniformLocation(this->program, uniformName);
            glUniform2fv(texPosUniform, 1, anim.frames[anim.currentFrame].texPos);
            sprintf(uniformName,"texInfo[%i].texSize", idx);
            GLuint texSizeUniform = glGetUniformLocation(this->program, uniformName);
            glUniform2fv(texSizeUniform, 1, anim.frames[anim.currentFrame].texSize);
            glUseProgram(0);
        }
        ++idx;
    }
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

void InstancedObject2D::updateInstanceType(int instance, bool enabled, Vec2 texPos)
{
    glUseProgram(this->program);
    char uniformName[50];
    sprintf(uniformName,"texInfo[%i].enabled", instance);
    GLuint enabledUniform = glGetUniformLocation(this->program, uniformName);
    glUniform1i(enabledUniform, enabled);
    if ( not enabled )
    {
        return;
    }
    sprintf(uniformName,"texInfo[%i].texPos", instance);
    GLuint texPosUniform = glGetUniformLocation(this->program, uniformName);
    float tpos[2] = {texPos.x, texPos.y};
    glUniform2fv(texPosUniform, 1, tpos);
    glUseProgram(0);
}

void InstancedObject2D::updateInstance(int instance, bool enabled, Vec2 pos, Vec2 texPos, Vec2 texSize)
{
    glUseProgram(this->program);
    char uniformName[50];
    sprintf(uniformName,"texInfo[%i].enabled", instance);
    GLuint enabledUniform = glGetUniformLocation(this->program, uniformName);
    glUniform1i(enabledUniform, enabled);
    if ( not enabled )
    {
        return;
    }
    sprintf(uniformName,"texInfo[%i].pos", instance);
    GLuint posUniform = glGetUniformLocation(this->program, uniformName);
    float vpos[2] = {this->pos.x + pos.x * this->size.x, this->pos.y + pos.y * this->size.y};
    //float vpos[2] = {pos.x, pos.y};
    glUniform2fv(posUniform, 1, vpos);
    sprintf(uniformName,"texInfo[%i].texPos", instance);
    GLuint texPosUniform = glGetUniformLocation(this->program, uniformName);
    float tpos[2] = {texPos.x, texPos.y};
    glUniform2fv(texPosUniform, 1, tpos);
    sprintf(uniformName,"texInfo[%i].texSize", instance);
    GLuint texSizeUniform = glGetUniformLocation(this->program, uniformName);
    float size[2] = {texSize.x, texSize.y};
    glUniform2fv(texSizeUniform, 1, size);
    glUseProgram(0);
}

void InstancedObject2D::draw()
{
    glUseProgram(this->program);

    glBindVertexArray(this->vao);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, this->numInstances);

    glBindVertexArray(0);

    glUseProgram(0);
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
    scene->currentLevel->draw(delta);

    scene->last = now;
    //printf("tick\n");
}


EntityID player;

void move(const Vec2i &dir)
{
    Bounds &b = s.getComponent<Bounds>(player);
    b.pos.x = dir.x;
    b.pos.y = dir.y;

/*
    if ( scene.objects.size() > 0 )
    {
        Object2D *obj = scene.objects[0];
        GLuint posUniform = glGetUniformLocation(obj->program, "texInfo[0].pos");
        glUseProgram(obj->program);
        glUniform2f(posUniform, dir.x, dir.y);
        glUseProgram(0);
        printf("move! position at %d\n", posUniform);
    }
    */
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

    InstancedObject2D *iobj = new InstancedObject2D;
    GLuint instancedProgram = createShader(loadText("shaders/simple.instanced.vs").c_str(), loadText("shaders/simple.instanced.fs").c_str());
    createInstancedObject(*iobj, instancedProgram);
    iobj->tex = obj->tex; // loadTexture("images/level.png",0);

    EntityID background = s.newEntity("background");
    //s.addComponent<Object2D*>(background,iobj);

    scene.currentLevel = new Level();

    s.showAll();

    startMainLoop(scene);

    //cleanScene(scene);

}