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
        exit(3);
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