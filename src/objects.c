Obj spo_camera(DVector2 pos)
{
    // NOTE: Default camera setup
    Camera2D cam = {
        .offset = Vector2Scale(ctx.res, 0.5),
        .target = Vector2Zero(),
        .rotation = 0.0f,
        .zoom = 1.0f,
    };

    return (Obj) {
        .id = spc_next_id(),
        .kind = OK_CAMERA,
        // NOTE: Cameras are enabled by default
        .enabled = true,
        .as = {
            .cam = (SPCamera) {
                .target = pos,
                .rl_cam = cam
            }
        }
    };
}

Obj spo_rect(DVector2 pos, DVector2 size, Color color)
{
    return (Obj) {
        .id = spc_next_id(),
        .kind = OK_RECT,
        .enabled = false,
        .as = {
            .rect = {
                .position = pos,
                .size = size,
                .color = color,
            }
        }
    };
}

Obj spo_text(const char *str, DVector2 pos, f32 font_factor, Color color)
{
    return (Obj) {
        .id = spc_next_id(),
        .kind = OK_TEXT,
        .enabled = false,
        .as = {
            .text = {
                .str = arena_strdup(&arena, str),
                .position = pos,
                // NOTE: I arbitrarily divided this by 2.f...it looked nicer
                .font_factor = font_factor / 2.f,
                .color = color,
            }
        }
    };
}

Obj spo_axes(Vector2 center, f32 xmin, f32 xmax, f32 ymin, f32 ymax)
{
    Vector2 size = {0.8, 0.8};
    Vector2 pos = Vector2Subtract(center, Vector2Scale(size, 0.5));

    pos = spv_denorm_coords(pos);
    size = spv_denorm_coords(size);

    Axes axes = {
        .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax,
        // .xmin = xmin, .xmax = xmax, .ymin = ymin, .ymax = ymax,
        .box = (Rectangle){
            .x = pos.x,
            .y = pos.y,
            .width = size.x,
            .height = size.y,
        },
    };
    // Coord of the center of the axes' box
    axes.center_coord = (Vector2) {
        .x = 0.5f * (axes.xmax + axes.xmin),
        .y = 0.5f * (axes.ymax + axes.ymin),
    };

    axes.coord_size = (Vector2) {
        .x = axes.box.width / (axes.xmax - axes.xmin),
        .y = axes.box.height / (axes.ymax - axes.ymin),
    };
    // axes.coord_size.y *= -1.f;

    // Position of the center of the axes' box
    Vector2 box_center = spv_denorm_coords(center);
    // Vector2 box_center = {
    //     .x = axes.box.x + 0.5f*axes.box.width,
    //     .y = axes.box.y + 0.5f*axes.box.height,
    // };

    Vector2 flip = {1.f, -1.f};
    axes.origin_pos = Vector2Add(
        box_center,
        Vector2Multiply(
            Vector2Negate(axes.center_coord),
            Vector2Multiply(axes.coord_size, flip)
        )
    );

    // axes.origin_pos = (Vector2) {
    //     .x = box_center.x + axes.center_coord.x * axes.coord_size.x,
    //     .x = box_center.x - axes.center_coord.x * axes.coord_size.x,
    // };

    return (Obj){
        .id = spc_next_id(),
        .kind = OK_AXES,
        .enabled = false,
        .as = { .axes = axes },
    };
}

Vector2 spo_curve_plot(const Axes *const axes, Vector2 pt)
{
    SP_ASSERT(axes->xmin <= axes->xmax);

    pt.y *= -1.f;
    Vector2 new_pt = Vector2Add(
        axes->origin_pos,
        Vector2Multiply(pt, axes->coord_size)
    );
    return new_pt;
}

Obj spo_curve(Id axes_id, UmkaCurvePts u_pts)
{
    Obj *axes_obj = NULL;
    spc_get_obj(axes_id, &axes_obj);
    SP_ASSERT(axes_obj->kind == OK_AXES);
    const Axes *axes = &axes_obj->as.axes;

    PointList pts = {0};
    int n = umkaGetDynArrayLen(&u_pts);
    for (int i = 0; i < n; i++) {
        Vector2 pt = spo_curve_plot(axes, spv_dtof(u_pts.data[i]));
        arena_da_append(&arena, &pts, pt);
    }

    return (Obj) {
        .id = spc_next_id(),
        .enabled = false,
        .kind = OK_CURVE,
        .as = {
            .curve = {
                .axes_id = axes_id,
                .pts = pts,
                .color = BLUE,
            }
        }
    };
}

Obj spo_typst(const char *text, f32 font_factor, DVector2 pos, Color color)
{
    Typst typ = {
        .text = text,
        // NOTE: I arbitrarily divided this by 42.5f...it looked nicer
        .font_factor = font_factor / 42.5f,
        .position = pos,
        .color = color,
    };
    spo_typst_compile(&typ);

    return (Obj){
        .id = spc_next_id(),
        .kind = OK_TYPST,
        .enabled = false,
        .as = { .typst = typ },
    };
}

bool spo_typst_compile(Typst *typ)
{
    if (!IsWindowReady()) {
        printf("Chillout bruv...\n");
        return false;
    }

    Nob_String_Builder sb = {0};
    f32 font_size = typ->font_factor * ctx.res.y;
    nob_sb_appendf(&sb,
        "#set page(width: auto, height: auto, margin: 0in, fill: none)\n"
        "#set text(size: %fpt, fill: white)\n"
        "$ %s $\n", font_size, typ->text);

    bool ok = nob_write_entire_file("input.typ", sb.items, sb.count);
    if (!ok) return false;
    nob_sb_free(sb);

    Nob_Cmd cmd = {0};
    const char *output_path = "output.png";
    nob_cmd_append(&cmd, "typst", "c", "input.typ", output_path);
    if (!nob_cmd_run_sync(cmd)) {
        printf("Failed to run command\n");
        return false;
    }

    typ->texture = LoadTexture(output_path);
    SetTextureFilter(typ->texture, TEXTURE_FILTER_BILINEAR);
    return true;
}

void spo_get_pos(Obj *obj, DVector2 **pos)
{
    switch (obj->kind) {
        case OK_RECT: {
            *pos = &obj->as.rect.position;
        } break;

        case OK_TEXT: {
            *pos = &obj->as.text.position;
        } break;

        case OK_TYPST: {
            *pos = &obj->as.typst.position;
        } break;

        case OK_CURVE: {
            *pos = &obj->as.curve.offset;
        } break;

        case OK_CAMERA: {
            *pos = &obj->as.cam.target;
        } break;

        default: {
            SP_UNREACHABLEF("Unknown kind of object: %d", obj->kind);
        } break;
    }
}

void spo_get_color(Obj *obj, Color **color)
{
    switch (obj->kind) {
        case OK_RECT: {
            *color = &obj->as.rect.color;
        } break;

        case OK_TEXT: {
            *color = &obj->as.text.color;
        } break;

        case OK_TYPST: {
            *color = &obj->as.typst.color;
        } break;

        case OK_CAMERA: {
            printf("WARN: A camera does not have a color; ignoring it.\n");
            SP_ASSERT(false);
        } break;

        default: {
            SP_UNREACHABLEF("Unknown kind of object: %d", obj->kind);
        } break;
    }
}

void spo_render(Obj obj)
{
    if (!obj.enabled) return;

    switch (obj.kind) {
        case OK_RECT: {
            Rect r = obj.as.rect;
            Vector2 pos = spv_denorm_coords(spv_dtof(r.position));
            Vector2 size = spv_denorm_coords(spv_dtof(r.size));
            pos = Vector2Subtract(pos, Vector2Scale(size, 0.5));

            DrawRectangleV(pos, size, r.color);
        } break;

        case OK_TEXT: {
            Text t = obj.as.text;
            Font font = GetFontDefault();
            f32 spacing = 2.0f;

            Vector2 pos = spv_denorm_coords(spv_dtof(t.position));
            f32 font_size = t.font_factor * ctx.res.y;
            Vector2 text_dim = MeasureTextEx(font, t.str, font_size, spacing);
            pos = Vector2Subtract(pos, Vector2Scale(text_dim, 0.5));

            DrawTextEx(font, t.str, pos, font_size, spacing, t.color);
        } break;

        case OK_AXES: {
            const Axes *axes = &obj.as.axes;
            Vector2 start, end;
            Color axes_color = WHITE;
            f32 thickness = 2.f;
            // // NOTE: Boundary rectangle - render only for debug purposes
            // DrawRectangleLinesEx(axes->box, 2.f, RED);

            if (axes->xmin <= 0.f && 0.f <= axes->xmax) {
                // Vertical axis
                start = (Vector2){axes->origin_pos.x, axes->box.y};
                end = (Vector2){axes->origin_pos.x, axes->box.y + axes->box.height};
                // start = Vector2Scale(start, ctx.scale_factor);
                // end = Vector2Scale(end, ctx.scale_factor);
                DrawLineEx(start, end, thickness, axes_color);
            }

            if (axes->ymin <= 0.f && 0.f <= axes->ymax) {
                // Horizontal axis
                start = (Vector2) {axes->box.x, axes->origin_pos.y};
                end = (Vector2) {axes->box.x + axes->box.width, axes->origin_pos.y};
                // start = Vector2Scale(start, ctx.scale_factor);
                // end = Vector2Scale(end, ctx.scale_factor);
                DrawLineEx(start, end, thickness, axes_color);
            }
        } break;

        case OK_CURVE: {
            Curve c = obj.as.curve;
            Vector2 offset = spv_denorm_coords(spv_dtof(c.offset));
            SP_ASSERT(c.pts.count >= 2);
            f32 thickness = 4.f;

            for (int i = 1; i < c.pts.count; i++) {
                Vector2 start = Vector2Add(c.pts.items[i-1], offset);
                Vector2 end = Vector2Add(c.pts.items[i], offset);

                // Line between two data points
                DrawLineEx(start, end, thickness, c.color);
                // Cap the first point to hide weird line artifacts
                DrawCircleV(start, thickness / 2.0f, c.color);
            }

            // Cap the last point with a circle
            DrawCircleV(
                Vector2Add(c.pts.items[c.pts.count - 1], offset),
                thickness / 2.0f, c.color);
        } break;

        case OK_TYPST: {
            Typst t = obj.as.typst;
            IVector2 tex_dim = {t.texture.width, t.texture.height};
            Vector2 pos = Vector2Subtract(
                spv_denorm_coords(spv_dtof(t.position)),
                Vector2Scale(spv_itof(tex_dim), 0.5));
            DrawTextureV(t.texture, pos, t.color);
        } break;

        // A camera can't be rendered
        case OK_CAMERA: break;

        default: {
            SP_UNREACHABLEF("Unknown kind of object: %d", obj.kind);
        } break;
    }
}

Action spo_enable(Id obj_id)
{
    return (Action) {
        .delay = 0.0,
        .obj_id = obj_id,
        .kind = AK_Enable,
        // NOTE: args should be left empty
    };
}

void spa_interp(Action action, void **value, f32 factor)
{
    factor = Clamp(factor, 0.0f, 1.0f);

    switch (action.kind) {
        case AK_Fade: {
            Color *c = *(Color**)value;
            FadeData args = action.args.fade;
            *c = ColorLerp(args.start, args.end, factor);
        } break;

        case AK_Move: {
            DVector2 *v = *(DVector2**)value;
            MoveData args = action.args.move;
            *v = spv_ftod(
                Vector2Lerp(spv_dtof(args.start), spv_dtof(args.end), factor)
            );
        } break;

        default: {
            SP_UNREACHABLEF("Unknown kind of action: %d", action.kind);
        } break;
    }
}

f32 sp_easing(f32 t, f32 duration)
{
    switch (ctx.easing) {
        case EM_Linear:
            return t / duration;

        case EM_Sine:
            return -0.5*cosf(PI / duration * t) + 0.5;

        default:
            SP_UNREACHABLEF("Unknown mode of easing: %d", ctx.easing);
    }
}
