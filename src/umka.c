#include "src/context.h"
typedef struct {
    Id id;
    Vector2 position, size;
    Color color;
} UmkaRect;

typedef struct {
    Id id;
    const char *text;
    Vector2 position;
    f32 font_size;
    Color color;
} UmkaText;

void spu_create_obj(UmkaStackSlot *p, UmkaStackSlot *r)
{
    ObjKind kind = *(ObjKind *)umkaGetParam(p, 0);
    void *obj = umkaGetParam(p, 1)->ptrVal;

    Id id = -1;
    switch (kind) {
        case OK_RECT: {
            UmkaRect *ur = obj;
            Obj rect = spo_rect(ur->position, ur->size, ur->color);
            id = rect.id;
            spc_add_obj(rect);
        } break;

        case OK_TEXT: {
            UmkaText *ut = obj;
            Obj text = spo_text(ut->text, ut->position, ut->font_size, ut->color);
            id = text.id;
            spc_add_obj(text);
        } break;

        default: NOB_UNREACHABLEF("Received: %d", kind);
    }

    umkaGetResult(p, r)->intVal = id;
}

Rect spu_convert_rect(UmkaRect ur)
{
    return (Rect) {
        .position = ur.position,
        .size = ur.size,
        .color = ur.color,
    };
}

Text spu_convert_text(UmkaText ut)
{
    return (Text) {
        .position = ut.position,
        .font_factor = ut.font_size,
        .text = ut.text,
        .color = ut.color,
    };
}

void spu_update_obj(UmkaStackSlot *p, UmkaStackSlot *r)
{
    NOB_UNUSED(r);
    Id id = *(Id *)umkaGetParam(p, 0);
    printf("id = %d\n", id);
    void *umka_obj = umkaGetParam(p, 1)->ptrVal;
    Obj *obj = NULL;
    if (!spc_get_obj(id, &obj)) {
        // TODO: add some log here
        return;
    }

    switch (obj->kind) {
        case OK_RECT: {
            UmkaRect *ur = umka_obj;
            obj->as.rect = spu_convert_rect(*ur);
        } break;

        case OK_TEXT: {
            UmkaText *ut = umka_obj;
            obj->as.text = spu_convert_text(*ut);
        } break;

        default: NOB_UNREACHABLEF("Received: %d", obj->kind);
    }
}

bool spu_init(const char *filename)
{
    Nob_String_Builder sb = {0};
    bool ok = spu_content_w_preamble(filename, &sb);
    if (!ok) {
        return false;
    }

    if (ctx.umka == NULL) {
        ctx.umka = umkaAlloc();
    }
    ok = umkaInit(ctx.umka, NULL, sb.items, 1024 * 1024, NULL, 0, NULL, false, false, NULL);
    if (!ok) {
        spu_print_err();
        return false;
    }

    UmkaFunc fns[] = {
        (UmkaFunc){.name = "create_obj", .func = &spu_create_obj},
        (UmkaFunc){.name = "update_obj", .func = &spu_update_obj},

        (UmkaFunc){.name = "fade_in", .func = &spu_fade_in},
        (UmkaFunc){.name = "play", .func = &spu_play},
    };
    for (size_t i = 0; i < NOB_ARRAY_LEN(fns); i++) {
        ok = umkaAddFunc(ctx.umka, fns[i].name, fns[i].func);
        if (!ok) {
            spu_print_err();
            return false;
        }
    }

    ok = umkaCompile(ctx.umka);
    if (!ok) {
        spu_print_err();
        return false;
    }
    return true;
}

void spu_run_sequence(void)
{
    spu_call_fn("sequence", NULL, 0);
}

void spu_preamble_count_lines(Nob_String_Builder sb)
{
    int count = 1;
    for (int i = 0; i < (int)sb.count; i++) {
        if (sb.items[i] == '\n') count++;
    }
    ctx.preamble_lines = count;
}

void spu_print_err(void)
{
    UmkaError *err = umkaGetError(ctx.umka);
    if (err->line <= ctx.preamble_lines) {
        printf("preamble:%d:%d: %s\n", err->line, err->pos, err->msg);
    } else {
        int line_no = err->line - ctx.preamble_lines;
        printf("%s:%d:%d: %s\n", err->fnName, line_no, err->pos, err->msg);
    }
}

bool spu_call_fn(const char *fn_name, UmkaStackSlot **slot, size_t storage_bytes)
{
    UmkaFuncContext fn = {0};
    bool umkaOk = umkaGetFunc(ctx.umka, NULL, fn_name, &fn);
    if (!umkaOk)
        return false;

    if (storage_bytes > 0) {
        umkaGetResult(fn.params, fn.result)->ptrVal = arena_alloc(&arena, storage_bytes);
    }

    umkaOk = umkaCall(ctx.umka, &fn) == 0;
    if (!umkaOk) {
        spu_print_err();
        return false;
    }

    // If slot is null, then I probably don't care about the result.
    // Just the fact that the function ran
    if (slot != NULL) {
        *slot = umkaGetResult(fn.params, fn.result);
    }
    return true;
}

bool spu_content_w_preamble(const char *filename, Nob_String_Builder *sb)
{
    *sb = (Nob_String_Builder){0};
    nob_read_entire_file("preamble.um", sb);
    spu_preamble_count_lines(*sb);

    Nob_String_Builder content_sb = {0};
    nob_read_entire_file(filename, &content_sb);

    nob_sb_appendf(sb, "\n%s", content_sb.items);
    return true;
}

#if 0
void spuo_rect(UmkaStackSlot *p, UmkaStackSlot *r)
{
    Vector2 pos = *(Vector2 *)umkaGetParam(p, 0);
    Vector2 size = *(Vector2 *)umkaGetParam(p, 1);
    Color color = *(Color *)umkaGetParam(p, 2);

    Obj rect = spo_rect(pos, size, color);
    umkaGetResult(p, r)->intVal = rect.id;

    spc_add_obj(rect);
}

void spuo_text(UmkaStackSlot *p, UmkaStackSlot *r)
{
    const unsigned char *text_str = (const unsigned char *)(umkaGetParam(p, 0)->ptrVal);
    Vector2 pos = *(Vector2 *)umkaGetParam(p, 1);
    f32 font_size = *(f32 *)umkaGetParam(p, 2);
    Color color = *(Color *)umkaGetParam(p, 3);

    Obj text = spo_text((const char *)text_str, pos, font_size, color);
    umkaGetResult(p, r)->intVal = text.id;

    spc_add_obj(text);
}

void spuo_axes(UmkaStackSlot *p, UmkaStackSlot *r)
{
    Vector2 center = *(Vector2 *)umkaGetParam(p, 0);
    f64 xmin = *(f64 *)umkaGetParam(p, 1);
    f64 xmax = *(f64 *)umkaGetParam(p, 2);
    f64 ymin = *(f64 *)umkaGetParam(p, 3);
    f64 ymax = *(f64 *)umkaGetParam(p, 4);

    Obj axes = spo_axes(spv_dtof(center), xmin, xmax, ymin, ymax);
    umkaGetResult(p, r)->intVal = axes.id;

    spc_add_obj(axes);
}

void spuo_curve(UmkaStackSlot *p, UmkaStackSlot *r)
{
    Id axes_id = *(Id *)umkaGetParam(p, 0);
    UmkaCurvePts umka_pts = *(UmkaCurvePts *)umkaGetParam(p, 1);

    Obj curve = spo_curve(axes_id, umka_pts);
    umkaGetResult(p, r)->intVal = curve.id;

    spc_add_obj(curve);
}

void spuo_typst(UmkaStackSlot *p, UmkaStackSlot *r)
{
    const unsigned char *text = (const unsigned char *)(umkaGetParam(p, 0)->ptrVal);
    f32 font_size = (f32)umkaGetParam(p, 1)->realVal;
    Vector2 pos = *(Vector2 *)umkaGetParam(p, 2);
    Color color = *(Color *)umkaGetParam(p, 3);

    Obj typst = spo_typst((const char *)text, font_size, pos, color);
    umkaGetResult(p, r)->intVal = typst.id;

    spc_add_obj(typst);
}

void spuo_get_camera(UmkaStackSlot *p, UmkaStackSlot *r)
{
    NOB_UNUSED(p);
    NOB_UNUSED(r);

    Id cam_id = 0;
    NOB_ASSERT(ctx.objs.items[cam_id].kind == OK_CAMERA);
    umkaGetResult(p, r)->intVal = cam_id;
}
#endif

void spuo_enable(UmkaStackSlot *p, UmkaStackSlot *r)
{
    NOB_UNUSED(r);
    Id obj_id = *(Id *)umkaGetParam(p, 0);

    spc_add_action(spo_enable(obj_id));
}

void spu_fade_in(UmkaStackSlot *p, UmkaStackSlot *r)
{
    NOB_UNUSED(r);

    Id obj_id = *(Id *)umkaGetParam(p, 0);
    f64 delay = *(f64 *)umkaGetParam(p, 1);

    Obj *obj = NULL;
    Color *current = NULL;
    NOB_ASSERT(spc_get_obj(obj_id, &obj));
    spo_get_color(obj, &current);

    FadeData fade = {
        .start = ColorAlpha(*current, 0.0),
        .end = ColorAlpha(*current, 1.0),
    };

    Action action = {
        .obj_id = obj_id,
        .delay = delay,
        .kind = AK_Fade,
        .args = {.fade = fade},
    };

    if (!obj->enabled) {
        // It need to be enabled first to be rendered on the screen.
        spc_add_action(spo_enable(obj_id));
    }
    spc_add_action(action);

    // Update the obj's prop
    *current = fade.end;
}

void spu_fade_out(UmkaStackSlot *p, UmkaStackSlot *r)
{
    NOB_UNUSED(r);

    Id obj_id = *(Id *)umkaGetParam(p, 0);
    f64 delay = *(f64 *)umkaGetParam(p, 1);

    Obj *obj = NULL;
    Color *current = NULL;
    NOB_ASSERT(spc_get_obj(obj_id, &obj));
    spo_get_color(obj, &current);

    FadeData fade = {
        .start = ColorAlpha(*current, 1.0),
        .end = ColorAlpha(*current, 0.0),
    };

    Action action = {
        .obj_id = obj_id,
        .delay = delay,
        .kind = AK_Fade,
        .args = {.fade = fade},
    };

    if (!obj->enabled) {
        // It need to be enabled first to be rendered on the screen.
        spc_add_action(spo_enable(obj_id));
    }
    spc_add_action(action);

    // Update the obj's prop
    *current = fade.end;
}

void spu_wait(UmkaStackSlot *p, UmkaStackSlot *r)
{
    NOB_UNUSED(p);
    NOB_UNUSED(r);
    Action action = {
        .obj_id = SCENE_OBJ,
        .delay = 0.0,
        .kind = AK_Wait,
        // NOTE: args should be left empty
    };
    spc_add_action(action);
}

void spu_move(UmkaStackSlot *p, UmkaStackSlot *r)
{
    NOB_UNUSED(r);

    Id obj_id = *(Id *)umkaGetParam(p, 0);
    Vector2 pos = *(Vector2 *)umkaGetParam(p, 1);
    f64 delay = *(f64 *)umkaGetParam(p, 2);

    Obj *obj = NULL;
    Vector2 *current = NULL;
    NOB_ASSERT(spc_get_obj(obj_id, &obj));
    spo_get_pos(obj, &current);

    MoveData move = {
        .start = *current,
        .end = pos,
    };

    Action action = {
        .obj_id = obj_id,
        .delay = delay,
        .kind = AK_Move,
        .args = {.move = move},
    };

    if (!obj->enabled) {
        // It need to be enabled first to be rendered on the screen.
        spc_add_action(spo_enable(obj_id));
    }
    spc_add_action(action);

    // Update the obj's prop
    *current = move.end;
}

void spu_play(UmkaStackSlot *p, UmkaStackSlot *r)
{
    NOB_UNUSED(r);
    f64 duration = *(f64 *)umkaGetParam(p, 0);

    Task *last = &ctx.tasks.items[ctx.tasks.count - 1];
    last->duration = duration;

    // NOTE: the line below is just a hack for now. a new task should only be added
    // when a new action is added. the line below just preemptively adds a task, which
    // is good but should not be done here.
    // TODO: remove this
    spc_new_task(0.0);
}
