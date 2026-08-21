> 与 Valve 无任何关联。
# Re : Aperture Terminal 2007
> 用 C++ 重现 2007 年托管在 [aperturescience.com](https://aperturescience.com) 的 Aperture Science Terminal。
## 简介
在《Portal》发售前后（2007），[aperturescience.com](https://aperturescience.com) 有一个 Aperture Science Terminal，它是基于 Flash 的，但是**Flash早就out了**。

本项目旨在用 C++ 重现它。
## 项目状态
几乎都完成了，还差两个彩蛋没有实现。

为了方便游玩，我们提供了简体中文版，详情见“构建”。
## 构建
g++就行,至少C++11，推荐使用更新的C++标准。

英语：`g++ -std=c++17 -O2 ReApertureTerminal.cpp -o ReApertureTerminal`\
简体中文版：`g++ ReApertureTerminal.cpp -o ReApertureTerminal -std=c++17 -O2 -DLANG_EN_US`
## 与原版的关系、素材与版权
本项目是一个非官方的社区重实现项目。\
原版 Aperture Science Terminal 于 2007 年通过 aperturescience.com 提供，并使用 Adobe Flash 构建。\
本项目的 C++ 源代码均为独立实现。Aperture Science Terminal 的原始文本以及基于原作衍生的文本均属于第三方内容，不在本项目 MIT 许可证的授权范围内。此类内容的相关权利仍归 Valve Corporation 或其相应权利人所有。\
本项目与 Valve Corporation、Aperture Science 或任何其他相关权利人均无关联，也未获得其认可或授权。\
本项目不主张拥有任何第三方名称、文本、角色、商标或其他相关内容的所有权。
## 许可证
本项目代码采用 [MIT License](LICENSE) 授权。

注意：MIT License 仅适用于本项目原创代码，
不适用于 Valve / Aperture Science 的名称、商标、原版文本、
音频、图像及其他第三方内容。
