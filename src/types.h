#ifndef _TYPES_H_
#define _TYPES_H_
#include "gl_headers.h"
#include <unordered_map>


typedef struct Vec2
{
    float x, y;
} Vec2;

typedef float Mat4[16];

typedef float Mat2[4];

typedef struct Vec2i
{
    int x, y;
    friend bool operator==(const Vec2i &a, const Vec2i &b)
    {
        return a.x == b.x && a.y == b.y;
    }
} Vec2i;

#define MAX_WIDTH 100

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
    Animation animation;
    virtual void update(float delta_s);
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
    std::unordered_map<Vec2i, int> instancePositions;
    virtual void draw() override;
    virtual void update(float delta_s) override;
    void updateInstance(int instance, bool enabled, Vec2 pos, Vec2 texPos, Vec2 texSize);
    void updateInstanceType(int instance, bool enabled, Vec2 texPos);
};

typedef struct {
    Vec2 speed;
    Vec2 acc;
} MotionParameters_t;

class Level;

typedef struct Scene2D
{
    bool running;
    #ifndef __EMSCRIPTEN__
    SDL_Window *window;
    #endif
    uint64_t last;
    Level *currentLevel;
} Scene2D;


typedef struct Action {
    
} Action;


#endif // _TYPES_H_