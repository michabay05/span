#ifndef _OBJECTS_H_
#define _OBJECTS_H_

typedef enum {
    AK_Enable,
    AK_Wait,
    AK_Fade,
    AK_Move,
} ActionKind;

typedef struct {
    Color start;
    Color end;
} FadeData;

typedef struct {
    DVector2 start;
    DVector2 end;
} MoveData;

typedef struct {
    Id obj_id;
    ActionKind kind;
    f64 delay;
    union {
        FadeData fade;
        MoveData move;
    } args;
} Action;
SP_STRUCT_ARR(ActionList, Action);

typedef struct {
    ActionList actions;
    f64 duration;
} Task;
SP_STRUCT_ARR(TaskList, Task);

typedef struct {
    DVector2 position;
    DVector2 size;
    Color color;
} Rect;

typedef struct {
    const char *str;
    DVector2 position;
    Vector2 norm_coords;
    // NOTE: Instead of storing what font size is in absolute terms (aka. in px),
    // the font size is described in terms of a multiplier. In other words, font
    // factor stores how many divisions tall the font is. Then, when rendering it
    // will be multiplied by the scale factor to obtain the absolute size.
    f32 font_factor;
    Color color;
} Text;

typedef struct {
    f64 xmin, xmax, ymin, ymax;
    Rectangle box;
    Vector2 origin_pos, coord_size, center_coord;
} Axes;

SP_STRUCT_ARR(PointList, Vector2);
// NOTE: Personally, I prefer that "children" don't possess any knowledge of
// their environment and who their "parent" is. But, for this one case, I
// will try to ignore that and give curves knowledge of their "parent": axes.
// The reason for this is because I want to allow curve to be treated as
// distinct sub-objects that can be enabled and disabled, so the easiest thing
// for me to do was to make a them a separate object (not sub-object).
typedef struct {
    Id axes_id;
    DVector2 offset;
    PointList pts;
    Color color;
} Curve;

typedef struct {
    const char *text;
    f32 font_factor;
    DVector2 position;
    Color color;
    Texture texture;
} Typst;

typedef struct {
    DVector2 target;
    Camera2D rl_cam;
} SPCamera;

typedef enum {
    OK_RECT,
    OK_TEXT,
    OK_AXES,
    OK_CURVE,
    OK_TYPST,
    OK_CAMERA,
} ObjKind;

typedef struct {
    Id id;
    ObjKind kind;
    bool enabled;
    union {
        Rect rect;
        Text text;
        Axes axes;
        Curve curve;
        Typst typst;
        SPCamera cam;
    } as;
} Obj;
SP_STRUCT_ARR(ObjList, Obj);

typedef enum {
    EM_Linear,
    EM_Sine,
} EaseMode;


Obj spo_camera(DVector2 pos);
Obj spo_rect(DVector2 pos, DVector2 size, Color color);
Obj spo_text(const char *str, DVector2 pos, f32 font_factor, Color color);
Obj spo_axes(Vector2 center, f32 xmin, f32 xmax, f32 ymin, f32 ymax);
Vector2 spo_curve_plot(const Axes *const axes, Vector2 pt);
Obj spo_curve(Id axes_id, UmkaCurvePts u_pts);
Obj spo_typst(const char *text, f32 font_factor, DVector2 pos, Color color);
bool spo_typst_compile(Typst *typ);
void spo_get_pos(Obj *obj, DVector2 **pos);
void spo_get_color(Obj *obj, Color **color);
void spo_render(Obj obj);
Action spo_enable(Id obj_id);
void spa_interp(Action action, void **value, f32 factor);
f32 sp_easing(f32 t, f32 duration);

#endif // _OBJECTS_H_
