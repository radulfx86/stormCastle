#include "object_factory.h"
#include "io.h"
#include "shaders.h"
#include "systems.h"
#include "state_machine.h"

EntityManager &s = EntityManager::getInstance();

void printObjectInfo(Object2D &obj)
{
    printf("Object2D(vao: %d vertexBuffer: %d, program: %d)\n",
    obj.vao, obj.vertexBuffer, obj.program);
}


void ObjectFactory::createInstancedObject(InstancedObject2D &obj, GLuint program)
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

EntityID ObjectFactory::addNPC(GLuint tex, GLuint program)
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
        Dialog2D *dialog = nullptr;
        if ( not s.hasComponent<Dialog2D*>(target) )
        {
            dialog = new Dialog2D;
            s.addComponent<Dialog2D*>(target,dialog);
        }
        dialog->active = true;

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

EntityID ObjectFactory::initBackground()
{
    InstancedObject2D *iobj = new InstancedObject2D;
    GLuint instancedProgram = createShader(loadText("shaders/simple.instanced.vs").c_str(), loadText("shaders/simple.instanced.fs").c_str());
    //createInstancedObject(*iobj, instancedProgram);
    ObjectFactory::createInstanceBackground(*iobj, instancedProgram);
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


EntityID ObjectFactory::addText(Vec2 pos, std::string content)
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

EntityID ObjectFactory::initPlayer(Object2D *obj)
{
    EntityID player = s.newEntity("player");
    s.addComponent<Object2D*>(player, obj);
    Bounds *bounds = new Bounds;
    bounds->pos = Vec2{1,1};
    bounds->size = Vec2{0.8,.4};
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

//void ObjectFactory::createObject(Object2D &obj, GLuint program, float tileSize = 1.0, Vec2 spriteSize = {1,1}, Vec2i spritePos = {0,0})
void ObjectFactory::createObject(Object2D &obj, GLuint program, float tileSize, Vec2 spriteSize, Vec2i spritePos)
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

Text2D *ObjectFactory::getText(Vec2 pos, std::string content, float color[3])
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

void ObjectFactory::createInstanceBackground(InstancedObject2D &obj, GLuint program)
{
    ObjectFactory::createObject(obj, program);
    obj.texOffset = 1;
    glUseProgram(obj.program);
    glUniform1i(glGetUniformLocation(obj.program, "tex"), 0);
    glUseProgram(0);
    obj.size = Vec2{1,1};
    obj.pos = Vec2{0,0};
    int i = 0;
    float w = 0.25;
    float h = 0.125;
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
