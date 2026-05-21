# span - types

## Scene
- A scene is composed of objects which will have actions performed on them for
  the duration for a certain period of time.
- The sum of all the actions' duration makes 100% of the scene's duration.

## Objects
- Objects are the representations of items shown on a given frame. For
  instance: rectangle, circle, images, etc.
- Objects are categorized based on who defines them. Some objects types are
  builtin while others are user-defined. Primitives are builtin objects, while
  composites are user-defined.
- All objects have the following common attributes, regardless of its type:
    - Enabled (on or off)
    - Position (px, py)
    - Rotation (about the z-axis)
    - Scale (sx, sy)
- In addition to the common attributes, each primitive type can have additional
  specific information that dictates how they are rendered.
    - Circle
        - radius
        - color
    - Rectangle
        - size (w, h)
        - color
    - Image
        - (image data)

## Actions
- Actions are instructions that change the state of a given object or groups of
  objects over a given period of time.
- Actions require the following in order to work as intended:
    - Attribute(s) being modified
    - Starting value of attribute(s)
    - Target value of attribute(s)
    - Duration
- Regardless of what is being modified, all actions will linearly interpolate
  between the start and end value using the duration provided.
- To allow for customizations, interpolation is not restricted to a linear one.
  Users can choose among existing interpolation mechanisms.
    - Builtin interpolations functions can be used, some of which include
        - Linear: $x / T$
        - Sinusoidal: $-0.5\left(\cos\left(\frac{\pi}{T}x\right)-1\right)$
    - (In the future) Users might be allowed to define their own interpolation functions.

## Presets

### Action Presets
- The language will have the following builtin actions or a preset of actions for convenience.
    - Wait (or sleep)
        - This is the equivalent of a no-op for a given period of time
    - Translate (or move)
    - Fade
    - Scale

### Widgets (aka. Object presets)
- Sometimes, basic objects along with their actions builtin make video development considerably more frictionless. To that end, some basic ones are builtin.
    - Some of them include but are not limited to
        - Timer
