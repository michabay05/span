#ifndef _CONTEXT_H_
#define _CONTEXT_H_

// NOTE: This is used for object associated with the wait action...essentially,
// this is to say the entire scene should just pause. It feels a hack, tho.
#define SCENE_OBJ ((Id)-1)

typedef enum {
    RM_Preview,
    RM_Output,
} RenderMode;

typedef struct {
    void *umka;

    // NOTE: this object list contains the original, unmodified state of the objects
    ObjList orig_objs;
    ObjList objs;

    TaskList tasks;
    Id id_counter;
    EaseMode easing;

    int preamble_lines;
    int current;
    f32 t;
    bool paused, quit, completed;

    Vector2 res;
    Camera2D cam;
    int fps;
    RenderMode render_mode;
    RenderTexture rtex;
    // NOTE: I'm not sure how to name this...essentially, if it is >= 1, dt
    // is multiplied by it. If it's < -1, then dt is divided by it. It can't be zero.
    int dt_mul;

    FFMPEG *ffmpeg;

    bool debug;
} Context;

Context ctx = {0};

bool spc_init(const char *filename, RenderMode mode);
void spc_renderer_init(RenderMode mode);
void spc_run_umka(void);
void spc_deinit(void);
void spc_update(f32 dt);
void spc_render(void);
void spc_add_obj(Obj obj);
Id spc_next_id(void);
void spc_print_tasks(TaskList tl);
void spc_new_task(f64 duration);
void spc_add_action(Action action);
bool spc_get_obj(Id id, Obj **obj);
void spc_reset(void);
// NOTE: For tasks, clear does not mean clear the list and free the memory.
void spc_clear_for_recomp(void);
Vector2 spv_dtof(DVector2 dv);
DVector2 spv_ftod(Vector2 v);
Vector2 spv_itof(IVector2 iv);
DVector2 spv_lerpd(DVector2 start, DVector2 end, f64 factor);
Vector2 spv_denorm_coords(Vector2 v);

#endif // _CONTEXT_H_
