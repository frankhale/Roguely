# Roguely

![Roguely Logo](assets/roguely-logo.png)

A simple Roguelike in SDL and C++

Test level showing off game sprites

![Roguely Logo](screenshots/sprite-sandbox.png)

## Status

NOTE: This is a work in progress...

The map is using simple cellular automata to generate it. Each time you run the
game you'll get a new map. Enemies spawn, move around and you can attack them to
increase your score. You can pick up health gems and coins. Dead enemies spawn a
treasure chest with a chance to increase health and score. If you kill a
`fire walker` he will spawn a treasure chest and an attack gem which increases
your attack power. Look for the golden candle and if you collect it you will
win the game.

## Next Steps

Before the code gets too messy, here is a list of things planned.

- Switch from manual dependency configuration to `vcpkg`
- Refactor current code so we can:
  - Integrate Lua
  - Finish ECS system
  - Move all in game objects (including player) to entities
  - Remove hard coded entity references that we have now
  - Optimize rendering pipeline
  - Expand the title screen to add a screen to document how the game is played
    and who the enemies are as well as what the objectives are.

In order to get there we have to figure out just how much Lua integration we'll
have and what that API will look like. The C++ side should provide the
foundation for the game but decisions about the game content should be exposed
from Lua. This is going to start shaping up over the next few weeks.

## Tileset

I've included the Photoshop file that was used to create the tileset. My
workflow is to develop tiles in 8x8 and then upscale them to 32x32 and then
export the tileset as a PNG which is used in the game.

## Screenshots

Title Screen:

![Title Screen](screenshots/title-screen.png)

Combat Text:

![Combat Text](screenshots/combat-text.png)

Current Gameplay:

We have cellular automata level generation and a minimap now

![New](screenshots/sixth.png)

Older gameplay showing how it evolved:

![Old](screenshots/fifth.png)

![Old](screenshots/fourth.png)

![Old](screenshots/third.png)

![Old](screenshots/second.png)

My first screenshot when I was able to render sprites:

![Old](screenshots/first.png)

## Videos

[![Game Play (Part 1)](https://img.youtube.com/vi/IOBuFlfgCSE/0.jpg)](https://www.youtube.com/watch?v=IOBuFlfgCSE)

## Building

I'm using `Visual Studio 2019 Community Preview Edition` but you can just use
regular `Visual Studio 2019 Community Edition`. The C++ project library/header
locations for SDL2 are set up the way I have it on my machine.

Get `Visual Studio 2019` from here: [https://visualstudio.microsoft.com/vs/](https://visualstudio.microsoft.com/vs/)

You can grab SDL2 from here (I'm using 64bit): [https://www.libsdl.org/download-2.0.php](https://www.libsdl.org/download-2.0.php)

You can grab the required libraries (I'm using 64bit) from here: [https://www.libsdl.org/projects/](https://www.libsdl.org/projects/)

I have put the 64 bit SDL2 Visual Studio lib/header files in the following
location:

![SDL2 Library Location](screenshots/required-libraries.png)

You'll need to copy the library DLLs to the root of the project folder (eg.
right where the source code is).

![SDL2 Required DLLs](screenshots/required-dlls.png)

If you set up your environment like this then you should be able to build
without touching the build settings. If you have trouble let me know and I'll
try to help.

Additionally Lua is part of the build process but is currently not being used.
The build expects that there is a 64 bit Lua library available to statically
link with the game. I don't want to remove this as I plan to use Lua
(along with Sol3) very soon.

![Lua](screenshots/lua.png)

![Lua](screenshots/lua2.png)

## License

MIT

## Credits for Audio

Music track `Exit Exit Proper - Pipe Choir` from:

- http://www.pipechoir.com/
- https://soundcloud.com/pipe-choir-three
- https://freemusicarchive.org/music/P_C_III

Creative Commons License: http://www.pipechoir.com/music-licenses.html

## Author(s)

Frank Hale &lt;frankhaledevelops@gmail.com&gt;

## Date

9 March 2021
