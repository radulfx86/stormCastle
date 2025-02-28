#include "types.h"
#include "tools.h"
/// TODO move into factories
#include "io.h"
#include "shaders.h"
#include "object_factory.h"

void Object2D::draw()
{
    //printf("drawObject: ");
    //printObjectInfo(obj);
    glUseProgram(this->program);

    glActiveTexture(GL_TEXTURE0);// + texOffset);
    printf("binding texture %d\n", this->tex);
    glBindTexture(GL_TEXTURE_2D, this->tex);

    glBindVertexArray(this->vao);

    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    glBindVertexArray(0);

    glUseProgram(0);
}

void Object2D::setAnimation(Direction_t animDir)
{
    animation.currentDirection = animDir;
    printf("setAnimation(%d)\n", animDir);
}

void Object2D::setPosition(Vec2 pos)
{
    Tools::validate(pos);
    this->pos = pos;
    printf("move to %2.2f %2.2f\n", pos.x, pos.y);
    float idMat[] = {1,0,0,0,
                    0,1,0,0,
                    0,0,1,0,
                    pos.x,pos.y,0,1};
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, idMat);
    glUseProgram(0);
}

void Object2D::updateAnimation(float delta_s)
{
    printf("animation direction: %d for object %p program %d\n", this->animation.currentDirection, this, program);
    bool updated = false;
    if ( this->animation.frames[this->animation.currentDirection].size() > 1 )
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
        printf("updated to frame %d dir %d at %0.2fx%0.2f size %0.2fx%0.2f\n",
               this->animation.currentFrame,
               this->animation.currentDirection,
               this->animation.frames[this->animation.currentDirection][this->animation.currentFrame].texPos[0],
               this->animation.frames[this->animation.currentDirection][this->animation.currentFrame].texPos[1],
               this->animation.frames[this->animation.currentDirection][this->animation.currentFrame].texSize[0],
               this->animation.frames[this->animation.currentDirection][this->animation.currentFrame].texSize[1]);
        glUseProgram(this->program);
        GLuint enabledUniform = glGetUniformLocation(this->program, "texInfo.flip");
        glUniform1i(enabledUniform, this->animation.frames[this->animation.currentDirection][this->animation.currentFrame].flip);
        GLuint posUniform = glGetUniformLocation(this->program, "texInfo.texOrigin");
        glUniform2fv(posUniform, 1, this->animation.frames[this->animation.currentDirection][this->animation.currentFrame].texOrigin);
        GLuint texPosUniform = glGetUniformLocation(this->program, "texInfo.texPos");
        glUniform2fv(texPosUniform, 1, this->animation.frames[this->animation.currentDirection][this->animation.currentFrame].texPos);
        GLuint texSizeUniform = glGetUniformLocation(this->program, "texInfo.texSize");
        glUniform2fv(texSizeUniform, 1, this->animation.frames[this->animation.currentDirection][this->animation.currentFrame].texSize);
        glUseProgram(0);
    }
}

void Object2D::updateCamera(Mat4 view, Mat4 proj)
{
    glUseProgram(this->program);
    glUniformMatrix4fv(glGetUniformLocation(this->program, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(this->program, "projection"), 1, GL_FALSE, proj);
    glUseProgram(0);
}

void InstancedObject2D::updateAnimation(float delta_s)
{
    int idx = 0;
    for ( Animation &anim : this->animations )
    {
        bool updated = false;
        if (anim.frames[this->animation.currentDirection].size() > 1)
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
                   anim.frames[this->animation.currentDirection][anim.currentFrame].texPos[0],
                   anim.frames[this->animation.currentDirection][anim.currentFrame].texPos[1],
                   anim.frames[this->animation.currentDirection][anim.currentFrame].texSize[0],
                   anim.frames[this->animation.currentDirection][anim.currentFrame].texSize[1]);
            glUseProgram(this->program);
            char uniformName[50];
            sprintf(uniformName,"texInfo[%i].flip", idx);
            GLuint enabledUniform = glGetUniformLocation(this->program, uniformName);
            glUniform1i(enabledUniform, anim.frames[this->animation.currentDirection][anim.currentFrame].flip);
            sprintf(uniformName,"texInfo[%i].texOrigin", idx);
            GLuint posUniform = glGetUniformLocation(this->program, uniformName);
            glUniform2fv(posUniform, 1, anim.frames[this->animation.currentDirection][anim.currentFrame].texOrigin);
            sprintf(uniformName,"texInfo[%i].texPos", idx);
            GLuint texPosUniform = glGetUniformLocation(this->program, uniformName);
            glUniform2fv(texPosUniform, 1, anim.frames[this->animation.currentDirection][anim.currentFrame].texPos);
            sprintf(uniformName,"texInfo[%i].texSize", idx);
            GLuint texSizeUniform = glGetUniformLocation(this->program, uniformName);
            glUniform2fv(texSizeUniform, 1, anim.frames[this->animation.currentDirection][anim.currentFrame].texSize);
            glUseProgram(0);
        }
        ++idx;
    }
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

void InstancedObject2D::updateInstanceTypePos(int instance, bool enabled, Vec2 pos, Vec2 texPos)
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
    printf("%s(%d, %f %f, %f %f, %f %f)\n",
        __func__, instance, pos.x, pos.y, texPos.x, texPos.y, texSize.x, texSize.y);
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
    glActiveTexture(GL_TEXTURE0);// + texOffset);
    printf("binding texture %d\n", this->tex);
    glBindTexture(GL_TEXTURE_2D, this->tex);

    glBindVertexArray(this->vao);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, this->numInstances);

    glBindVertexArray(0);

    glUseProgram(0);
}

void Text2D::setText(std::string text)
{
    int oldNumInstances = numInstances;
    
    Vec2 pos{this->pos.x, this->pos.y};
    int idxTextOut = 0;
    for ( size_t idxText = 0; idxText < text.size(); ++idxText )
    {
        pos.x += this->characterDisplayDistance.x;
        if ( text[idxText] == '\n' || text[idxText] == '\r' )
        {
            pos.x = this->pos.x;
            pos.y -= this->characterDisplayDistance.y;
            continue;
        }
        //InstancedObject2D::updateInstanceType(textIndex[text[idxText]], true, Vec2{0,0});
        int idxChar = textIndex[text[idxText]];
        Vec2 texPos{(float)(idxChar % this->textureColumns), (float)(idxChar / this->textureColumns) };
        printf("set Text %d '%c' -> %d t: %f %f p: %f %f\n", idxTextOut, text[idxText], idxChar, texPos.x, texPos.y, pos.x, pos.y);
        InstancedObject2D::updateInstance(idxTextOut, true, pos, texPos, this->characterSize);
        ++idxTextOut;
    }
    numInstances = idxTextOut;
    for ( int i = numInstances; i < oldNumInstances && i < 255; ++i )
    {
        InstancedObject2D::updateInstanceType(i, false, Vec2{0,0});
    }
}

void Text2D::setCharacterSize(Vec2 size, Vec2 displayDistance = {1,1})
{
    this->characterSize = size;
    this->characterDisplayDistance = displayDistance;
}

void Text2D::setColor(float color[4])
{
    glUseProgram(this->program);
    glUniform4fv(glGetUniformLocation(this->program, "textColor"), 1, color);
    glUseProgram(0);
}

void Path2D::updateCamera(float view[16], float proj[16])
{
    glUseProgram(this->program);
    glUniformMatrix4fv(glGetUniformLocation(this->program, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(this->program, "projection"), 1, GL_FALSE, proj);
    glUseProgram(0);
}

void Path2D::setPosition(Vec2 pos)
{
    Tools::validate(pos);
    printf("move to %2.2f %2.2f\n", pos.x, pos.y);
    float idMat[] = {1,0,0,0,
                    0,1,0,0,
                    0,0,1,0,
                    pos.x,pos.y,0,1};
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, idMat);
    glUseProgram(0);
}

void Path2D::draw()
{
    glUseProgram(this->program);

    glBindVertexArray(this->vao);

    glDrawArrays(GL_LINE_STRIP,0,vertexData.size());

    glBindVertexArray(0);

    glUseProgram(0);
}

Path2D::Path2D(std::vector<Vec2> elements, float color[3])
{
    (void)color;
    program = createShader(loadText("shaders/dbg_path.vs").c_str(), loadText("shaders/dbg_path.fs").c_str());
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vertexBuffer);
    for (const Vec2 v : elements)
    {
        vertexData.push_back(v.x);
        vertexData.push_back(v.y);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexData.size(), &vertexData[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float idMat[] = {1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1};
    float idMat_01[] = {.125f, 0, 0, 0,
                        0, .125f, 0, 0,
                        0, 0, .125f, 0,
                        0, 0, 0, 1};
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, idMat);
    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, idMat_01);
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, idMat);
    glUniform3fv(glGetUniformLocation(program, "color"), 1, color);
    glUseProgram(0);
}

void Path2D::setPath(std::vector<Vec2> elements)
{
    for (const Vec2 v : elements)
    {
        vertexData.push_back(v.x);
        vertexData.push_back(v.y);
    }
    glBindVertexArray(vao);

    for (const Vec2 v : elements)
    {
        vertexData.push_back(v.x);
        vertexData.push_back(v.y);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexData.size(), &vertexData[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Path2D::setColor(float color[3])
{
    glUseProgram(program);
    glUniform3fv(glGetUniformLocation(program, "color"), 1, color);
    glUseProgram(0);
}

#include "controller.h"

Dialog2D::Dialog2D()
{
    float green[] = {0.3,1.0,0.2};
    this->text = ObjectFactory::getText(Vec2{0,0},"test",green);
    this->bg = ObjectFactory::createSimpleBgObject(1.0, Vec2{1,1}, Vec2i{0,0});
    this->bg->texOffset = 0;
    this->bg->tex = loadTexture("assets/images/bad_scribble_1.png", 0);
    this->controller = new DialogController(this->tree, this->source, this->target); 
}

void Dialog2D::draw()
{
    this->bg->draw();
    // meh
    this->text->setText("bla");
    this->text->draw();
}

void Dialog2D::setPosition(Vec2 pos)
{
    this->bg->setPosition(pos);
    this->text->setPosition(pos);
}

void Dialog2D::updateCamera(Mat4 view, Mat4 proj)
{
    exit(3);
    (void)view;
    (void)proj;
    //this->text->updateCamera(view, proj);
    this->text->updateCamera(view, proj);
    this->bg->updateCamera(view,proj);
}

#if 0

TexObject2D::TexObject2D()
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

}

void TexObject2D::draw()
{
    // meh
    this->text->setText("bla");
    this->text->draw();
}

void TexObject2D::setPosition(Vec2 pos)
{
    (void)pos;
}
void TexObject2D::updateCamera(Mat4 view, Mat4 proj)
{
    (void)view;
    (void)proj;
    //this->text->updateCamera(view, proj);
}
#endif