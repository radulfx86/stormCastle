#ifndef _TOOLS_H_
#define _TOOLS_H_

#include "types.h"

namespace Tools
{
    template <typename T>
    void validate(T el);
    bool doesIntersect(Bounds * sourceBounds, Bounds * targetBounds);
    bool inBounds(const Bounds * bounds, const Vec2 pos);
    Vec2 dirVector(Direction_t dir);
    Direction_t dirFromVector(Vec2i dir);
} // namespace Tools

#endif // _TOOLS_H_