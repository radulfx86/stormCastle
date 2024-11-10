#include "types.h"

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
void Object2D::updateAnimation(float delta_s)
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

void InstancedObject2D::updateAnimation(float delta_s)
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