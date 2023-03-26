[**English**](./README.md)  | **中文**

![github license](https://img.shields.io/github/license/Dozingfiretruck/nes)![linux](https://github.com/Dozingfiretruck/nes/actions/workflows/action.yml/badge.svg?branch=master)



# nes

#### 介绍
c语言实现的nes模拟器，要求c11

**注意：**

**本仓库仅为nes模拟器，不提供游戏本体！！！**

支持情况：

- [x] CUP

- [x] PPU

- [ ] APU

mapper 支持：0，2

#### 软件架构
示例基于SDL2进行图像声音输出，没有特殊依赖，您可自行移植至任意硬件


#### 编译教程

​	克隆本仓库，安装[xmake](https://github.com/xmake-io/xmake)  ，直接执行 xmake 编译即可 

#### 使用说明

​	linux下输入 `./nes xxx.nes` 加载要运行的游戏
​	windows下输入 `.\nes.exe xxx.nes` 加载要运行的游戏



#### 按键映射

| 手柄 |  上  |  下  |  左  |  左  | 选择 | 开始 |  A   |  B   |
| :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
|  P1  |  W   |  S   |  A   |  D   |  V   |  B   |  J   |  K   |
|  P2  |  ↑   |  ↓   |  ←   |  →   |  1   |  2   |  5   |  6   |

#### 运行展示

![super_mario](./doc/super_mario.png)

![contra](./doc/contra.png)


#### 文献参考

https://www.nesdev.org/



