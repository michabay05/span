bool spc_init(const char *filename, RenderMode mode)
{
    // NOTE: The initialization goes through two steps:
    //   (1) Umka: compile script, load objs and actions
    //   (2) Raylib: initialize window and other related things

    ctx = (Context){0};
    ctx.paused = false;
    ctx.completed = false;
    ctx.easing = EM_Sine;
    ctx.dt_mul = 1;

    bool ok = spu_init(filename);
    if (!ok) return false;

    spc_renderer_init(mode);

    spc_run_umka();
    return true;
}

void spc_renderer_init(RenderMode mode)
{
    IVector2 pres = { 800, 600 };
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(pres.x, pres.y, "span");
    ctx.fps = 60;
    SetTargetFPS(ctx.fps);

    switch (mode) {
        case RM_Preview: {
            ctx.res = spv_itof(pres);

            if (pres.x < pres.y) {
                // The horz length is smaller (Portrait)
                SP_UNIMPLEMENTED("Portrait mode is not implemented yet!");
            }
        } break;

        case RM_Output: {
            ctx.res = (Vector2){ 1200, 900 };
            // NOTE: The aspect ratio of the preview window and video have to be the same.
            // TODO: This is a temporary limitation; fix this
            f32 p_aspect_ratio = (f32)pres.x / (f32)pres.y;
            f32 v_aspect_ratio = (f32)ctx.res.x / (f32)ctx.res.y;
            SP_ASSERT(p_aspect_ratio == v_aspect_ratio);

            if (ctx.res.x < ctx.res.y) {
                // The horz length is smaller (Portrait)
                SP_UNIMPLEMENTED("Portrait mode is not implemented yet!");
            }

            ctx.rtex = LoadRenderTexture(ctx.res.x, ctx.res.y);
            SetTextureFilter(ctx.rtex.texture, TEXTURE_FILTER_BILINEAR);
            ctx.ffmpeg = ffmpeg_start_rendering_video(
                "out.mov", (size_t)ctx.res.x, (size_t)ctx.res.y, (size_t)ctx.fps);
        } break;

        default: {
            SP_UNREACHABLEF("Unknown render mode: %d", mode);
        } break;
    }
    ctx.render_mode = mode;

    // Add main camera
    // NOTE: the main camera should always be the first object in the obj_list
    spc_add_obj(
        spo_camera(spv_ftod(Vector2Zero()))
    );
}

void spc_run_umka(void)
{
    spu_run_sequence();
    spc_reset();
}

void spc_deinit(void)
{
    umkaFree(ctx.umka);

    if (ctx.render_mode == RM_Output) {
        ffmpeg_end_rendering(ctx.ffmpeg, false);
        UnloadRenderTexture(ctx.rtex);
    }
    CloseWindow();

    arena_free(&arena);
}

void spc_update(f32 dt)
{
    if (ctx.paused || ctx.completed) {
        return;
    }

    if (ctx.current >= ctx.tasks.count) {
        ctx.completed = true;
        if (ctx.render_mode == RM_Output) ctx.quit = true;
        return;
    }

    Task task = ctx.tasks.items[ctx.current];
    float factor = sp_easing(ctx.t, task.duration);

    if (ctx.t > task.duration) {
        ctx.current++;
        ctx.t = 0.0f;
    }

    ActionList al = task.actions;
    for (int i = 0; i < al.count; i++) {
        Action a = al.items[i];

        switch (a.kind) {
            case AK_Enable: {
                Obj *obj = NULL;
                SP_ASSERT(spc_get_obj(a.obj_id, &obj));
                obj->enabled = true;
            } break;

            case AK_Wait: break;

            case AK_Move: {
                Obj *obj = {0};
                DVector2 *pos = NULL;
                SP_ASSERT(spc_get_obj(a.obj_id, &obj));
                SP_ASSERT(obj->enabled);
                spo_get_pos(obj, &pos);
                SP_ASSERT(pos != NULL);

                spa_interp(a, (void*)&pos, factor);
            } break;

            case AK_Fade: {
                Obj *obj = {0};
                Color *color = NULL;
                SP_ASSERT(spc_get_obj(a.obj_id, &obj));
                SP_ASSERT(obj->enabled);
                spo_get_color(obj, &color);
                SP_ASSERT(color != NULL);

                spa_interp(a, (void*)&color, factor);
            } break;

            default: {
                SP_UNREACHABLEF("Unknown kind: %d", a.kind);
            } break;
        }
    }

    ctx.t += dt;
}

static void spc__main_render(void)
{
    // NOTE: The first object in the list should always be camera
    Obj obj = ctx.objs.items[0];
    SP_ASSERT(obj.kind == OK_CAMERA);
    Camera2D cam = obj.as.cam.rl_cam;
    cam.target = spv_denorm_coords(spv_dtof(obj.as.cam.target));

    ClearBackground(BLACK);

    BeginMode2D(cam); {
        for (int i = 0; i < ctx.objs.count; i++) {
            Obj *obj = NULL;
            SP_ASSERT(spc_get_obj(i, &obj));
            spo_render(*obj);
        }
    } EndMode2D();
}

static void spc__preview_render(void)
{
    BeginDrawing(); {
        spc__main_render();

        IVector2 pos = {10, 10};
        DrawFPS(pos.x, pos.y);
        DrawText(
            TextFormat(ctx.dt_mul > 0 ? "%dx" : "1/%dx", abs(ctx.dt_mul)),
            pos.x, pos.y + 25, 20, WHITE
        );
        const char *text = NULL;
        if (ctx.completed) {
            text = "Completed";
        } else if (ctx.paused) {
            text = "Paused";
        } else {
            text = "Playing";
        }
        if (ctx.debug) {
            text = TextFormat("%s [D]", text);
        }
        DrawText(text, pos.x, pos.y + 2*25, 20, WHITE);
    } EndDrawing();
}

static void spc__output_render(void)
{
    // Render to the render texture
    BeginTextureMode(ctx.rtex); {
        spc__main_render();

        SetTraceLogLevel(LOG_WARNING);
        Image image = LoadImageFromTexture(ctx.rtex.texture);
        SetTraceLogLevel(LOG_INFO);
        if (!ffmpeg_send_frame(ctx.ffmpeg, image.data, image.width, image.height)) {
            ffmpeg_end_rendering(ctx.ffmpeg, true);
        }
        UnloadImage(image);
    } EndTextureMode();

    // Render to preview window
    BeginDrawing(); {
        f32 font_size = 40;
        f32 spacing = 2.0;
        Font font = GetFontDefault();

        const char *text = "Rendering...";
        Vector2 text_dim = MeasureTextEx(font, text, font_size, spacing);
        Vector2 preview_res = {GetScreenWidth(), GetScreenHeight()};
        Vector2 pos = Vector2Scale(preview_res, 0.5);
        pos = Vector2Subtract(pos, Vector2Scale(text_dim, 0.5));

        DrawTextEx(font, text, pos, font_size, spacing, WHITE);
    } EndDrawing();
}

void spc_render(void)
{
    switch (ctx.render_mode) {
        case RM_Preview: {
            spc__preview_render();
        } break;

        case RM_Output: {
            spc__output_render();
        } break;

        default: {
            SP_UNREACHABLEF("Unknown render mode: %d", ctx.render_mode);
        } break;
    }
}

void spc_add_obj(Obj obj)
{
    arena_da_append(&arena, &ctx.objs, obj);
    arena_da_append(&arena, &ctx.orig_objs, obj);
}

Id spc_next_id(void)
{
    Id id = ctx.id_counter;
    ctx.id_counter++;
    return id;
}

void spc_print_tasks(TaskList tl)
{
    printf("%d\n", tl.count);
    for (int i = 0; i < tl.count; i++) {
        Task t = tl.items[i];
        printf("Task {\n");
        printf("    duration = %f\n", t.duration);
        for (int k = 0; k < t.actions.count; k++) {
            Action a = t.actions.items[k];
            printf("    [%2d] {id = %d, kind = %d, delay = %f}\n", k, a.obj_id, a.kind, a.delay);
        }
        printf("}\n");
    }
}

void spc_new_task(f64 duration)
{
    arena_da_append(&arena, &ctx.tasks, (Task){.duration = duration});
}

void spc_add_action(Action action)
{
    if (ctx.tasks.count == 0) {
        // NOTE: Added a task here to make sure the code below will have at
        // least one task to attach the action to.
        spc_new_task(0.0);
    }

    Task *last = &ctx.tasks.items[ctx.tasks.count - 1];
    arena_da_append(&arena, &last->actions, action);
}

bool spc_get_obj(Id id, Obj **obj)
{
    if (id < ctx.objs.count) {
        *obj = &ctx.objs.items[id];
        return true;
    } else {
        return false;
    }
}

void spc_reset(void)
{
    for (int i = 0; i < ctx.objs.count; i++) {
        ctx.objs.items[i] = ctx.orig_objs.items[i];
    }
    ctx.current = 0;
    ctx.t = 0.0f;
    ctx.paused = false;
    ctx.completed = false;
    ctx.quit = false;
}

// NOTE: For tasks, clear does not mean clear the list and free the memory.
void spc_clear_for_recomp(void)
{
    // NOTE (cont.): It just means that I will move the count variable
    // to zero so as to write new tasks over the old ones. The memory
    // will cleaned up at the end.
    ctx.tasks.count = 0;

    for (int i = 0; i < ctx.objs.count; i++) {
        Obj *o = &ctx.objs.items[i];
        switch (o->kind) {
            case OK_TYPST: {
                UnloadTexture(o->as.typst.texture);
            } break;

            default: break;
        }
    }
}

Vector2 spv_dtof(DVector2 dv)
{
    return (Vector2){
        .x = (f32)dv.x,
        .y = (f32)dv.y,
    };
}

DVector2 spv_ftod(Vector2 v)
{
    return (DVector2){
        .x = (f64)v.x,
        .y = (f64)v.y,
    };
}

Vector2 spv_itof(IVector2 iv)
{
    return (Vector2){
        .x = (f32)iv.x,
        .y = (f32)iv.y,
    };
}

DVector2 spv_lerpd(DVector2 start, DVector2 end, f64 factor)
{
    if (factor < 0.0) factor = 0.0;
    if (factor > 1.0) factor = 1.0;

    return (DVector2) {
        .x = start.x + (end.x - start.x)*factor,
        .y = start.y + (end.y - start.y)*factor,
    };
}

Vector2 spv_denorm_coords(Vector2 v)
{
    Vector2 factor = ctx.res;
    f32 aspect_ratio = ctx.res.x / ctx.res.y;
    factor.x = ctx.res.x / aspect_ratio;
    return Vector2Multiply(v, factor);
}
