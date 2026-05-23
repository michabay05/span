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

TaskInfo :: struct($T: typeid) {
    start, end: T,
    value: ^T
}
Task :: struct {
    data: []union { TaskInfo(f32), TaskInfo(Vec2) },
    t, duration: f32
}

interp_f32 :: proc(task: ^TaskInfo(f32), t, duration: f32) {
    task.value^ = task.start + (task.end - task.start) * min(t / duration, 1.)
}
interp_vec2 :: proc(task: ^TaskInfo(Vec2), t, duration: f32) {
    task.value^ = task.start + (task.end - task.start) * min(t / duration, 1.)
}
interp_task :: proc(task: ^Task, dt: f32) {
    for td in task.data {
        switch &task_info in td {
        case TaskInfo(f32) : interp_f32(&task_info, task.t, task.duration)
        case TaskInfo(Vec2): interp_vec2(&task_info, task.t, task.duration)
        case: unimplemented()
        }
    }
    task.t += dt
}

main :: proc() {
    rl.SetTraceLogLevel(.WARNING)
    rl.SetConfigFlags({.MSAA_4X_HINT})
    rl.InitWindow(800, 600, "span - window")
    defer rl.CloseWindow()

    rl.SetTraceLogLevel(.INFO)
    rl.SetTargetFPS(60)

    ob1 := Circle_Data {
        enabled = true,
        position = {},
        radius = 30
    }
    // Line 68
    task := []Task {
        {
            data = {TaskInfo(f32) {
                start = ob1.radius,
                end = 60,
                value = &ob1.radius,
            }},
            duration = 1.
        },
        {
            data = {TaskInfo(Vec2) {
                start = ob1.position,
                end = {300, 200},
                value = &ob1.position,
            }},
            duration = 0.5
        }
    }

    camera := rl.Camera2D {
        offset = ({f32(rl.GetScreenWidth()), f32(rl.GetScreenHeight())}) / 2.,
        target = {},
        rotation = 0.,
        zoom = 1.,
    }
    // Line 94

    for !rl.WindowShouldClose() {
        camera.zoom = math.exp_f32(
            math.ln_f32(camera.zoom) + f32(rl.GetMouseWheelMove()) * 0.1)
        if rl.IsMouseButtonDown(.LEFT) {
            camera.target += rl.GetMouseDelta() * -1./camera.zoom
        }

        interp_task(&task[0], rl.GetFrameTime())
        interp_task(&task[1], rl.GetFrameTime())

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
        rl.DrawFPS(10, 10)
        rl.EndDrawing()
    }
}
