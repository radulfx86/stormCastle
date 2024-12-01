#ifndef _OBJECT_FACTORY_H_
#define _OBJECT_FACTORY_H_

#include "types.h"

namespace ObjectFactory
{

void createInstancedObject(InstancedObject2D &obj, GLuint program);

EntityID addNPC(GLuint tex, GLuint program);

EntityID initBackground();

EntityID addText(Vec2 pos, std::string content);

EntityID initPlayer(Object2D *obj);

//void createObject(Object2D &obj, GLuint program, float tileSize, Vec2 spriteSize, Vec2i spritePos);
void createObject(Object2D &obj, GLuint program, float tileSize = 1.0, Vec2 spriteSize = {1,1}, Vec2i spritePos = {0,0});

Text2D *getText(Vec2 pos, std::string content, float color[3]);

void createInstanceBackground(InstancedObject2D &obj, GLuint program);

Object2D *createSimpleBgObject(float tileSize, Vec2 spriteSize, Vec2i spritePos);

} // namespace ObjectFactory

#endif // _OBJECT_FACTORY_H_