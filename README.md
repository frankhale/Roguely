# Roguely

![Roguely Logo](assets/roguely-logo.png)

A simple Roguelike in SDL/C++/Lua

Test level showing off game sprites. The spritesheet is coming along for a much
richer game but there is still a lot of work still to do. This screenshot **does
not denote actual actual game play**

![Roguely Logo](screenshots/sprite-sandbox.png)

## Status

NOTE: This is a work in progress...

The game so far is very simple anbd is using cellular automata to generate maps.
Each time you run the game you'll get a new map. Enemies spawn, move around
randomly and you can attack them to increase your score. You can pick up health
gems and coins. Dead enemies spawn a treasure chest with a chance to increase
health and score. Look for the golden candle and if you collect it you will win
the game.

## Next Steps

This concluded during rewrite.

- [x] Switch from manual dependency configuration to `vcpkg`
- [x] Refactor current code so we can:
  - [x] Integrate Lua
  - [x] Finish ECS system
  - [x] Move all in game objects (including player) to entities
  - [x] Remove hard coded entity references that we have now
  - [x] Optimize rendering pipeline
  - [ ] Expand the title screen to add a screen to document how the game is played and who the enemies are as well as what the objectives are.

## Tileset

I've included the Photoshop file that was used to create the tileset. My
workflow is to develop tiles in 8x8 and then upscale them to 32x32 and then
export the tileset as a PNG which is used in the game.

The tileset is sort of all over the place. It's not well organized and has
ventured into some animation tiles. We aren't doing any animation yet and a lot
of these tiles are not currently being used. The main focus so far has been to
get a basic engine up, provide lua support and get something working and for
that I am very happy and recognize that there is still a long way to go before
a proper game can be made.

## Game Objections (so far)

Not a lot to do besides run around and kill enemies and increase score. If you
run across the golden candle you win. It's highlighted in yellow on the mini
map. If the level generation spawned you in a blocked area just hit the space
bar to warp to another location.

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

[![Game Play (Part 2)](https://img.youtube.com/vi/Bs1GXWLNYok/0.jpg)](https://www.youtube.com/watch?v=Bs1GXWLNYok)

[![Game Play (Part 1)](https://img.youtube.com/vi/IOBuFlfgCSE/0.jpg)](https://www.youtube.com/watch?v=IOBuFlfgCSE)

## Building

I'm using `Visual Studio 2019 Community Preview Edition` but you can just use
regular `Visual Studio 2019 Community Edition` to build this code.

Get `Visual Studio 2019` from here: [https://visualstudio.microsoft.com/vs/](https://visualstudio.microsoft.com/vs/)

I'm using `vcpkg` for dependencies management. You can get `vckpg` here: [https://github.com/Microsoft/vcpkg](https://github.com/Microsoft/vcpkg)

Roguely has the following dependencies:

- SDL2
- SDL_image
- SDL_mixer (with mpg123 support)
- SDL_ttf
- Lua
- Sol2
- Boost (using boost/uuid, boost/numeric)

## License

MIT

## Credits for Audio

Music track `Exit Exit Proper - Pipe Choir` from:

- [http://www.pipechoir.com/](http://www.pipechoir.com/)
- [https://soundcloud.com/pipe-choir-three](https://soundcloud.com/pipe-choir-three)
- [https://freemusicarchive.org/music/P_C_III](https://freemusicarchive.org/music/P_C_III)

Creative Commons License: http://www.pipechoir.com/music-licenses.html

The sounds in the `assets/sounds` folder came from [https://opengameart.org/](https://opengameart.org/)

## Author(s)

Frank Hale &lt;frankhaledevelops@gmail.com&gt;

## Date

22 March 2021
