**English** | [中文](./README_zh.md) 

![github license](https://img.shields.io/github/license/Dozingfiretruck/nes)![linux](https://github.com/Dozingfiretruck/nes/actions/workflows/windows.yml/badge.svg?branch=master)![linux](https://github.com/Dozingfiretruck/nes/actions/workflows/linux.yml/badge.svg?branch=master)![linux](https://github.com/Dozingfiretruck/nes/actions/workflows/macos.yml/badge.svg?branch=master)



# nes

## Introduction
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

mapper  support：0, 2, 3, 7, 94, 117, 180

## Software Architecture
The example is based on SDL2 for image and sound output, without special dependencies, and you can port to any hardware by yourself


## Compile Tutorial

​	clone repository，install [xmake](https://github.com/xmake-io/xmake)，execute `xmake` directly to compile

### Compile Preparation

#### Windows:	

​	install MSVC([Visual Studio 2022](https://visualstudio.microsoft.com/zh-hans/vs/))

​	install [xmake](https://github.com/xmake-io/xmake)

#### Linux(Ubuntu):

```shell
sudo add-apt-repository ppa:xmake-io/xmake -y
sudo apt-get update -y
sudo apt-get install -y git make gcc p7zip-full libsdl2-dev xmake
```

#### Macox:

```shell
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
brew update
brew install make gcc sdl2 xmake
```

### Compilation Method

​	clone repository，execute `xmake` directly to compile

## Instructions

​	on linux or macos enter  `./nes xxx.nes` load the game to run

​	on windows enter `.\nes.exe xxx.nes` load the game to run



## Key mapping

| joystick |  up  | down | left | right | select | start |  A   |  B   |
| :------: | :--: | :--: | :--: | :---: | :----: | :---: | :--: | :--: |
|    P1    | `W`  | `S`  | `A`  |  `D`  |  `V`   |  `B`  | `J`  | `K`  |
|    P2    | `↑`  | `↓`  | `←`  |  `→`  |  `1`   |  `2`  | `5`  | `6`  |

## showcase

**mapper 0:**

| ![Super Mario Bros](./docs/SuperMarioBros.png) | ![F1_race](./docs/F1_race.png) | ![Star Luster (J)](./docs/StarLuster(J).png) | ![Ikki (J)](./docs/Ikki(J).png) |
| :--------------------------------------------: | :----------------------------: | :------------------------------------------: | ------------------------------- |
|  ![Circus Charlie](./docs/CircusCharlie.png)   |                                |                                              |                                 |

**mapper 2:**


|  ![Contra1](./docs/Contra1.png)  | ![Castlevania](./docs/Castlevania.png) | ![Journey](./docs/Journey.png) | ![Lifeporce](./docs/Lifeporce.png) |
| :------------------------------: | :------------------------------------: | :----------------------------: | ---------------------------------- |
| ![mega_man](./docs/mega_man.png) |  ![Athena (J)](./docs/Athena(J).png)   |                                |                                    |

**mapper 3:**

| ![contra](./docs/MapleStory.png) | ![Donkey_kong](./docs/Donkey_kong.png) |
| :------------------------------: | :------------------------------------: |



**mapper 94:**

![Senjou no Ookami](./docs/Senjou_no_Ookami(J).png)

**mapper 180:**

![Crazy Climber](./docs/CrazyClimber(J).png)

## Literature reference

https://www.nesdev.org/



