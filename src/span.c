#define ARENA_IMPLEMENTATION
#define NOB_IMPLEMENTATION

#include "base.h"
#include "umka.h"
#include "objects.h"
#include "context.h"

#include "ffmpeg_linux.c"
#include "umka.c"
#include "objects.c"
#include "context.c"

int main(void)
{
    const char *filename = "./test.um";
#if 1
    bool success = spc_init(filename, RM_Preview);
#else
    bool success = spc_init(filename, RM_Output);
#endif
    if (!success) return 1;

    while (!ctx.quit && !WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            ctx.paused = !ctx.paused;
        }
        if (IsKeyPressed(KEY_LEFT_SHIFT)) {
            ctx.dt_mul--;
            if (ctx.dt_mul == 0) ctx.dt_mul = -2;
        }
        if (IsKeyPressed(KEY_RIGHT_SHIFT)) {
            ctx.dt_mul++;
            if (ctx.dt_mul == -1) ctx.dt_mul = 1;
        }
        if (IsKeyPressed(KEY_H)) {
            spc_reset();
            printf("Restarted animation\n");
        }
        if (IsKeyPressed(KEY_C)) {
            spc_clear_for_recomp();
            spu_init(filename);
            spc_run_umka();
            printf("Recompiled %s\n", filename);
        }
        if (IsKeyPressed(KEY_D)) {
            ctx.debug = !ctx.debug;
        }

        if (!ctx.paused) {
            SP_ASSERT(ctx.dt_mul != 0);
            f32 mult = ctx.dt_mul > 0 ? (f32)ctx.dt_mul : 1.0 / (f32)abs(ctx.dt_mul);
            spc_update(GetFrameTime() * mult);
        }

        spc_render();
    }

    spc_deinit();

    return 0;
}
