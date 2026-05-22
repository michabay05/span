package span

import "core:math"

import rl "vendor:raylib"
Vec2 :: rl.Vector2

BaseObject :: struct {
    enabled: bool,
    position: Vec2,
}

Rect_Data :: struct {
    using base: BaseObject,
    size: Vec2,
}

Circle_Data :: struct {
    using base: BaseObject,
    radius: f32,
}

Object :: union {
    Rect_Data,
    Circle_Data
}

Task :: struct($T: typeid) {
    start, end: T,
    value: ^T,
    t, duration: f32
}

interp_f32 :: proc(task: ^Task(f32), dt: f32) {
    task.value^ = task.t >= task.duration ? task.end : task.start + (task.end - task.start) * (task.t / task.duration)
    task.t += dt
}
interp_vec2 :: proc(task: ^Task(Vec2), dt: f32) {
    task.value^ = task.t >= task.duration ? task.end : task.start + (task.end - task.start) * (task.t / task.duration)
    task.t += dt
}
interp :: proc{interp_vec2, interp_f32}

main :: proc() {
    rl.SetTraceLogLevel(.WARNING)
    rl.SetConfigFlags({.MSAA_4X_HINT})
    rl.InitWindow(800, 600, "span - window")
    defer rl.CloseWindow()

    rl.SetTraceLogLevel(.INFO)
    rl.SetTargetFPS(60)

    ob1 := Rect_Data {
        enabled = true,
        position = {},
        size = {40, 30}
    }
    task := Task(Vec2) {
        start = ob1.position,
        end = {150, 150},
        value = &ob1.position,
        t = 0.,
        duration = 1.
    }

    camera := rl.Camera2D {
        offset = ({f32(rl.GetScreenWidth()), f32(rl.GetScreenHeight())}) / 2.,
        target = {},
        rotation = 0.,
        zoom = 1.,
    }

    t: f32 = 0.
    for !rl.WindowShouldClose() {
        camera.zoom = math.exp_f32(
            math.ln_f32(camera.zoom) + f32(rl.GetMouseWheelMove()) * 0.1)
        if rl.IsMouseButtonDown(.LEFT) {
            camera.target += rl.GetMouseDelta() * -1./camera.zoom
        }

        interp(&task, rl.GetFrameTime())

        rl.BeginDrawing()
        rl.ClearBackground(rl.GetColor(0x181818ff))
        rl.BeginMode2D(camera)
        switch ob in Object(ob1) {
        case Rect_Data:
            rec := rl.Rectangle { ob.position.x, ob.position.y, ob.size.x, ob.size.y }
            rl.DrawRectanglePro(rec, ob.size / 2., 0., rl.RED)

        case Circle_Data:
            rl.DrawCircleV(ob1.position, ob.radius, rl.BLUE)
        }
        rl.EndMode2D()
        rl.EndDrawing()
    }
}
