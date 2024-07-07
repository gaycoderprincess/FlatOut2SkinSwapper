# FlatOut 2 Skin Swapper

Plugin to load car textures from loose files in FlatOut 2

![ingame preview](https://i.imgur.com/wPFTYnE.png)
![filesystem preview](https://i.imgur.com/YUdDtLs.png)

## Installation

- Make sure you have v1.2 of the game, as this is the only version this plugin is compatible with. (exe size of 2990080 bytes)
- Plop the files into your game folder, edit `FlatOut2SkinSwapper_gcp.toml` to change the options to your liking.
- Place car textures into the same folder structure as you would if the game was unpacked. (You can use .dds, .tga and .png files)
- Enjoy, nya~ :3

## Building

Building is done on an Arch Linux system with CLion and vcpkg being used for the build process. 

Before you begin, clone [nya-common](https://github.com/gaycoderprincess/nya-common) to a folder next to this one, so it can be found.

Required packages: `mingw-w64-gcc`

You should be able to build the project now in CLion.
