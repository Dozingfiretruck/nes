**English** | [中文](./README_zh.md) 

![github license](https://img.shields.io/github/license/Dozingfiretruck/nes)![linux](https://github.com/Dozingfiretruck/nes/actions/workflows/windows.yml/badge.svg?branch=master)![linux](https://github.com/Dozingfiretruck/nes/actions/workflows/linux.yml/badge.svg?branch=master)![linux](https://github.com/Dozingfiretruck/nes/actions/workflows/macos.yml/badge.svg?branch=master)



# nes

#### Introduction
The nes simulator implemented in c , requires c11

**attention：**

**This repository is only for the nes simulator and does not provide the game ！！！**

Platform support:

- [x] Windows

- [x] Linux

- [x] MacOS

Simulator support：

- [x] CPU

- [x] PPU

- [x] APU

mapper  support：0, 2, 3

#### Software Architecture
The example is based on SDL2 for image and sound output, without special dependencies, and you can port to any hardware by yourself


#### Compile Tutorial

​	clone repository，install [xmake](https://github.com/xmake-io/xmake)，execute `xmake` directly to compile

#### Instructions

​	on linux or macos enter  `./nes xxx.nes` load the game to run

​	on windows enter `.\nes.exe xxx.nes` load the game to run



#### Key mapping

| joystick |  up  | down | left | right | select | start |  A   |  B   |
| :------: | :--: | :--: | :--: | :---: | :----: | :---: | :--: | :--: |
|    P1    |  W   |  S   |  A   |   D   |   V    |   B   |  J   |  K   |
|    P2    |  ↑   |  ↓   |  ←   |   →   |   1    |   2   |  5   |  6   |

#### showcase

mapper 0:

|   Game    |                  showcase                  |
| :---------: | :------------------------------------: |
| Super Mario | ![super_mario](./docs/super_mario.png) |
|   F1 race   |     ![F1_race](./docs/F1_race.png)     |


mapper 2:

| Game |             showcase             |
| :----: | :--------------------------: |
| Contra1 | ![Contra1](./docs/Contra1.png) |
| 恶魔城1 | ![Castlevania](./docs/Castlevania.png) |
| 西游记1 |   ![Journey](./docs/Journey.png)   |
| mega man | ![mega_man](./docs/mega_man.png) |
| Lifeporce | ![Lifeporce](./docs/Lifeporce.png) |

mapper 3:

|   Game   |               showcase               |
| :--------: | :------------------------------: |
| MapleStory | ![contra](./docs/MapleStory.png) |
| Donkey kong | ![Donkey_kong](./docs/Donkey_kong.png) |

#### Literature reference

https://www.nesdev.org/



