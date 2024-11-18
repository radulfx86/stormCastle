#ifndef _TYPES_H_
#define _TYPES_H_
#include "gl_headers.h"
#include <unordered_map>
#include <vector>


typedef struct Vec2
{
    float x, y;
    friend Vec2 operator+(const Vec2 &a, const Vec2 &b)
    {
        return Vec2{a.x+b.x, a.y+b.y};
    }
} Vec2;

typedef float Mat4[16];

typedef float Mat2[4];

#define MAX_WIDTH 100

typedef struct Vec2i
{
    int x, y;
    friend bool operator==(const Vec2i &a, const Vec2i &b)
    {
        return a.x == b.x && a.y == b.y;
    }
    friend bool operator<(const Vec2i &a, const Vec2i &b)
    {
        return (a.x * MAX_WIDTH + a.y) < (b.x * MAX_WIDTH + b.y);
        //return a.x * a.x + a.y * a.y < b.x * b.x + b.y * b.y;
    }
} Vec2i;


template<>
struct std::hash<Vec2i>
{
    std::size_t operator()(const Vec2i &v) const noexcept
    {
        return v.x * MAX_WIDTH + v.y;
    }
};

typedef struct Bounds
{
    Vec2 pos;
    Vec2 size;
} Bounds;

typedef struct TexInfo
{
    float texPos[2];
    float texSize[2];
    bool flip;
    float texOrigin[2];
} TexInfo;

typedef struct Animation {
    std::vector<TexInfo> frames;
    std::vector<float> deltas;
    int currentFrame;
    float currentDelta;
} Animation;

class Object2D
{
public:
    GLuint attribPos;
    GLuint vao;
    GLuint vertexBuffer;
    GLuint tex;
    GLuint program;
    GLuint texOffset;
    Animation animation;
    virtual void updateAnimation(float delta_s);
    virtual void draw();
    virtual void setPosition(Vec2 pos);
};

class InstancedObject2D : public Object2D
{
public:
    int numInstances;
    Vec2 pos;
    Vec2 size;
    std::vector<Animation> animations;
    /// @note might not be needed
    std::unordered_map<Vec2i, int> instancePositions;
    virtual void draw() override;
    virtual void updateAnimation(float delta_s) override;
    void updateInstance(int instance, bool enabled, Vec2 pos, Vec2 texPos, Vec2 texSize);
    void updateInstanceType(int instance, bool enabled, Vec2 texPos);
    void updateInstanceTypePos(int instance, bool enabled, Vec2 pos, Vec2 texPos);
};

typedef struct {
    Vec2 speed;
    Vec2 acc;
} MotionParameters_t;

typedef struct {
    bool active;
    Vec2 direction;
    enum { NONE, TALK, TAKE, GIVE, TRIGGER, MOVE, MELEE, RANGED, NUM_INTERACTIONS } type;
} InteractionParameters_t;

typedef void (*TriggerFunction)(int target, int source);

class Level;

class Controller;

typedef struct Scene2D
{
    bool running;
    #ifndef __EMSCRIPTEN__
    SDL_Window *window;
    #endif
    uint64_t last;
    Level *currentLevel;
    Controller *controller;
    uint64_t tick;
} Scene2D;


typedef struct Action {
   enum { IDLE = 0, MOTION, INTERACT, ATTACK, TALK, SPECIAL, NUM_ACTIONS} TYPE;
   Vec2i v2iParam;
   Vec2 v2fParam;
   bool bParam;
} Action;


#endif // _TYPES_H_