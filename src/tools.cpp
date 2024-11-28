#include "tools.h"

#include "types.h"
#include <signal.h>
#include <stdio.h>

template<>
void Tools::validate(Vec2 el)
{
    if ( abs(el.x) > 1E5 || abs(el.y) > 1E5 )
    {
        raise(SIGABRT);
        printf("Vec2 not valid!!!\n");
        exit(7);
    }
}

template<>
void Tools::validate(Object2D *el)
{
    if ( 0 == el )
    {
        raise(SIGABRT);
        printf("0 pointer !!!\n");
        exit(4);
    }
}

template<>
void Tools::validate(Object2D el)
{
    (void) el;
    /// TODO
}

bool Tools::doesIntersect(Bounds * sourceBounds, Bounds * targetBounds)
{
    return (sourceBounds && targetBounds) &&
        (inBounds(sourceBounds, targetBounds->pos) 
         || inBounds(sourceBounds, Vec2{targetBounds->pos.x, targetBounds->pos.y + targetBounds->size.y}) 
         || inBounds(sourceBounds, Vec2{targetBounds->pos.x + targetBounds->size.x, targetBounds->pos.y + targetBounds->size.y}) 
         || inBounds(sourceBounds, Vec2{targetBounds->pos.x + targetBounds->size.x, targetBounds->pos.y})
         || inBounds(targetBounds, sourceBounds->pos) 
         || inBounds(targetBounds, Vec2{sourceBounds->pos.x, sourceBounds->pos.y + sourceBounds->size.y}) 
         || inBounds(targetBounds, Vec2{sourceBounds->pos.x + sourceBounds->size.x, sourceBounds->pos.y + sourceBounds->size.y}));
}
bool Tools::inBounds(const Bounds * bounds, const Vec2 pos)
{
    bool isInBounds = bounds &&  
        ((pos.x > bounds->pos.x) && (pos.x < (bounds->pos.x+bounds->size.x)))
        && ((pos.y > bounds->pos.y) && (pos.y < (bounds->pos.y+bounds->size.y)));
    return isInBounds;
}

Vec2 Tools::dirVector(Direction_t dir)
{
    switch(dir)
    {
        case UP:
            return Vec2{0,-1};
        case DOWN:
            return Vec2{0,1};
        case LEFT:
            return Vec2{-1,0};
        case RIGHT:
            return Vec2{1,0};
        default:
            return Vec2{0,0};
    }
}

Direction_t Tools::dirFromVector(Vec2i vec)
{
    if ( vec.x >= 1.0 )
    {
        return RIGHT;
    }
    if ( vec.x <= -1.0 )
    {
        return LEFT;
    }
    if ( vec.y >= 1.0 )
    {
        return DOWN;
    }
    if ( vec.y <= -1.0 )
    {
        return UP;
    }
}
