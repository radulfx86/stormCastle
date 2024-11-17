#ifndef _DISPLAY_H_
#define _DISPLAY_H_
#include "gl_headers.h"
#include "types.h"
#include "controller.h"

void mainloop(void *userData);

void move(const Vec2i &dir);

#ifdef __EMSCRIPTEN__

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
    printf("mouse clicked at %d %d\n", e->clientX, e->clientY);
    return 0;
}

EM_BOOL touch_callback(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
    printf("mouse clicked at %d %d\n", e->touches[0].clientX, e->touches[0].clientY);
    return 0;
}

EM_BOOL keydown_callback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
    Scene2D *scene = (Scene2D *)userData;
    switch (keyEvent->keyCode)
    {
    case 0x1B:  // escape
        scene->running = false;
        break;
    case 0x27:  // right arrow
        scene->controller->addAction(Action{Action::MOTION, Vec2i{1, 0}, {0}, false});
        break;
    case 0x25:  // left arrow
        scene->controller->addAction(Action{Action::MOTION, Vec2i{-1, 0}, {0}, false});
        break;
    case 0x26:  // up arrow
        scene->controller->addAction(Action{Action::MOTION, Vec2i{0, 1}, {0}, false});
        break;
    case 0x28:  // down arrow
        scene->controller->addAction(Action{Action::MOTION, Vec2i{0, -1}, {0}, false});
        break;
    case 0x20:  // space
        scene->controller->addAction(Action{Action::INTERACT, {0}, {0}, false});
        break;
    default:
        break;
    }
    return 0;
}

void initScene(Scene2D &scene)
{
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);

    attrs.enableExtensionsByDefault = 1;
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;

    emscripten_set_canvas_element_size("#canvas", 640, 480);
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    if (!context)
    {
        printf("failed to create context\n");
        exit(1);
    }

    emscripten_webgl_make_context_current(context);

    printf("context\n");

    printf("setup callbacks\n");

    emscripten_set_click_callback("#canvas", 0, 1, mouse_callback);
    emscripten_set_touchend_callback("#canvas", 0, 1, touch_callback);
    emscripten_set_keydown_callback("#canvas", &scene, 1, keydown_callback);
}

void startMainLoop(Scene2D &scene)
{
    emscripten_set_main_loop_arg(mainloop, &scene, 0, 0);
}

uint64_t getNow()
{
    return (uint64_t) emscripten_performance_now();
}
#else
uint64_t getNow()
{
    return SDL_GetTicks64();
}
void handleInput(Scene2D &scene)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                scene.running = false;
                break;
            case SDLK_RIGHT:
                scene.controller->addAction(Action{Action::MOTION, Vec2i{1,0}, {0}, event.type == SDL_KEYDOWN});
                break;
            case SDLK_LEFT:
                scene.controller->addAction(Action{Action::MOTION, Vec2i{-1,0}, {0}, event.type == SDL_KEYDOWN});
                break;
            case SDLK_UP:
                scene.controller->addAction(Action{Action::MOTION, Vec2i{0,1}, {0}, event.type == SDL_KEYDOWN});
                break;
            case SDLK_DOWN:
                scene.controller->addAction(Action{Action::MOTION, Vec2i{0,-1}, {0}, event.type == SDL_KEYDOWN});
                break;
            case SDLK_SPACE:
                scene.controller->addAction(Action{Action::INTERACT, {0}, {0}, event.type == SDL_KEYDOWN});
                break;
            default:
                break;
            }
        }
    }
}
void startMainLoop(Scene2D &scene)
{
    scene.running = true;
    while ( scene.running )
    {
        handleInput(scene);
        mainloop(&scene);
        //if ( scene.tick % 10 == 1)
        SDL_GL_SwapWindow(scene.window);
    }
}
void initScene(Scene2D &scene)
{

    scene.running = false;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    scene.window = SDL_CreateWindow("meh", 0, 0, 512, 512, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(scene.window);
    (void) context;
}

#endif

void cleanScene(Scene2D &scene)
{
}

#endif // _DISPLAY_H_
