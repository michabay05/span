Originally written by: @michabay05 (Dec 27, 2024)


## Philosophy
The animation engine consists of two different parts, namely the executable and the library.

The library, written in Python, will be used to create a file, which contains all the information required for the executable to create the animation. In other words, interpreting the python file will be used to create the anim file, which, in turn, will be parsed by the executable. All the changes would only need to be made to the python file, which will remain relatively simple to iteratively improve. This approach significantly speeds up the animation development process because the animation isn't compiled into the executable.

## Usage
```console
> $(EXEC_PATH) --help
Usage:
    $(EXEC_PATH) preview [OPTIONS] <ANIM_FILE>
    $(EXEC_PATH) render [OPTIONS] <ANIM_FILE>

Options:
    . . .
```

## File format structure
There are 4 sections with each section beginning with `<%...%>`. The first section contains the definition of important constants used throughout the animation. The second section outlines a list of all the unique objects used throughout the animation. Following it, there's an optional section where groups are listed. These grouped objects contain a list of some objects, which will have some form of animation applied to them simultaneously. Lastly, all the animations will be listed below.
```
<%CONST_DEFS%>
. . .
<%OBJ_DEFS%>
. . .
<%GROUP_DEFS%>
. . .
<%ANIM_DEFS%>
. . .
```

## Required CONSTANT definitions
```
(float) FONT_SIZE_TO_HEIGHT = 2.50f;
(float) UNIT_TO_PX = 25.0f;
(uint)  ANIM_START_INDEX = 0;
```

## OBJECT definitions
```
Line {
    id: uint,
    start: Vector2,
    end: Vector2,
    thickness: float,
    color: Color,
}

Circle {
    id: uint,
    center: Vector2,
    radius: float,
    fill_color: Color,
    stroke_width: float,
    stroke_color: Color,
    stroke_segments: int
}

Rectangle {
    id: uint,
    top_left: Vector2,
    size: Vector2,
    fill_color: Color,
    stroke_width: float,
    stroke_color: Color,
}

Tex {
    id: uint,
    text: str,
    font_size: float,
    position: Vector2,
    stroke_width: float,
    stroke_color: Color,
}
```

## Supported Actions
- List of actions supported
    - FadeIn
    - FadeOut
    - ColorFade
    - PositionTransform
    - Scale
    - SwapPosition
    - SwapColor
    - Create
    - PauseScene

These are some animations planned to be implemented. ([Source](https://manimclass.com/manim-animation-types/))

