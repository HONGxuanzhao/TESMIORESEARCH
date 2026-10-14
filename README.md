# TESMIORESEARCH

[GitHub](https://github.com/HONGxuanzhao/TESMIORESEARCH) | [Steam Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=3814026714)

## Download and installation / 下载与安装

The ready-to-use plugin files are in `plugins/`. Download this repository using **Code → Download ZIP**, extract it, and follow the instructions below.

可直接使用的插件文件位于 `plugins/`。点击 **Code → Download ZIP** 下载并解压，按下文安装。

## Package instructions / 使用说明

TESMIORESEARCH — TESMIO / Republic Mod Loader 安装说明
====================================================

运行包结构
README.txt、previewimage.png 放在根目录；插件 DLL、INI 和 RML 元数据放在 plugins 目录。
本包面向 WRSR 1.1.1.9（64 位）。DLL 使用 TESMIO API 4；RML 使用其 TESMIO 兼容宿主加载。

TESMIO 安装
1. 完全退出游戏，从本包 plugins 目录复制 TESMIORESEARCH.dll 和 TESMIORESEARCH.ini 到：
   <游戏目录>\tesmioloader\build\plugins\（tesmioloader.dll 旁的 plugins 目录）。
2. 启用 TESMIO 插件扫描；若 [plugins] 显式关闭了 TESMIORESEARCH，将其设为 1。
3. 使用 tesmiolauncher.exe 启动。RML 的 .rml.json 元数据无需复制给 TESMIO。

Republic Mod Loader（RML）安装
1. 订阅并下载对应创意工坊项目后，使用支持 TESMIO API 4 的 RML 启动器。
2. 刷新列表，在 Plugins 中启用 TESMIORESEARCH；工坊文件应位于 <项目编号>\plugins\。
3. 手动安装时，将本包 plugins 内的 DLL、INI、.rml.json 复制到 RML 的 rml\plugins\。
4. INI 与实际加载的 DLL 放在同一目录。若已有手动安装副本，先备份个人设置再统一安装来源。

配置与发布
编辑实际加载目录内的 INI；保存为 UTF-8 无 BOM，并完整重启游戏。
同一个游戏进程选择一个 Loader 启动。可选的 Reputation 联动不构成强制依赖。
上传目录应包含 plugins、README.txt、previewimage.png 及该项目原有的 workshopconfig.ini。
保留各项目的 ITEM_ID、OWNER_ID 和可见性，使用 SCRIPT 项目类型；不要上传传输 ZIP。
本次调整验证了打包结构和接口；调整参数后的游戏效果及存档兼容性仍需实际运行验证。

English installation
====================
Target: WRSR 1.1.1.9 x64; TESMIO API 4 or RML with the TESMIO API 4 compatibility host.
TESMIO: copy TESMIORESEARCH.dll and TESMIORESEARCH.ini from this package's plugins folder into
<game>\tesmioloader\build\plugins, enable the plugin, and launch with tesmiolauncher.exe.
RML Workshop: subscribe/download the item, refresh the inventory and enable TESMIORESEARCH.
RML manual: copy DLL, INI and .rml.json from plugins into RML's rml\plugins folder.
Keep the INI beside the DLL that is actually loaded. Edit settings, then fully restart.
Use one loader per game process. Reputation integration is optional, not a required dependency.
Publish the plugins folder with root README/preview and the item's original workshopconfig.ini;
preserve item/owner IDs and visibility, use SCRIPT type, and do not upload the transfer ZIP.
Package/interface checks do not establish gameplay behavior or save compatibility.

功能与参数 / Features and settings
====================================================
TESMIORESEARCH 0.3.1 — 原版科技数值调节 / Original Research Controls

中文
需要 TESMIO API 4；本地目标版本 WRSR 1.1.1.9 / TESMIO b0.3.6。
将 DLL、INI 放入 TESMIO 的 build\plugins，通过 TESMIO 启动；修改 INI 后完整重启。
不需要其他功能插件；[plugins] 中 TESMIORESEARCH 应为 1。

[research] enabled：1 开启，0 关闭。
speed_multiplier：全局研究速度倍率，范围 0.05—20。
[technical]、[medical]、[soviet] speed_multiplier：技术、医学、社会分支倍率。
[research.原版科技ID] speed_multiplier：单项研究倍率。
三个倍率相乘，COST = 基础工作量 ÷ 最终倍率，四舍五入且至少 1。
COST 是研究工作量，实际历时还取决于研究人员和效率，不是固定现实秒数。

[effects]
pollution_filter_remaining：完成原版污染过滤研究后的排放剩余系数；范围 0—1，原值 0.775。
policy_duration_multiplier：原版 LOCK_AFTER_DAYS 政策期限倍率；范围 0.05—20，默认 1。
housing_height_threshold：住宅解锁高度阈值；范围 1—500，原值 33.8。
housing_quality_threshold：住宅解锁品质阈值；范围 0.1—1，原值 0.899999。
住宅参数调整的是研究解锁门槛，不是住宅实际品质或建筑高度。

已移除新增科技、布局钩子和新增科技完成加成。原版科技 ID、顺序、图标和前置条件由游戏保留。
旧 INI 的 expanded_tree 等扩展设置不再生效。
沿用原版/VFS 读取链，在临时文件中修改数值，不覆盖游戏数据或直接编辑存档。
污染参数仅在偏离原值时启用版本/指令校验的专用读取补丁；冲突时保留原系数。
默认所有倍率为 1，其他数值为原值，默认配置不改变原版数值。
旧扩展科技存档的迁移仍需单独验证，请保留原存档备份。

English
Requires TESMIO API 4; local target: WRSR 1.1.1.9 / TESMIO b0.3.6.
Copy DLL/INI to build\plugins, enable TESMIORESEARCH, and launch through TESMIO.
Restart after editing settings. No other feature plugin is required.

[research] enabled: 1 on, 0 off. speed_multiplier controls global research speed.
[technical], [medical], [soviet] use speed_multiplier for each original branch.
[research.original_ID] speed_multiplier overrides a single existing research.
Speed factors multiply; COST is rounded(base workload / speed), minimum 1.
Workload does not specify a fixed number of elapsed days or real seconds.

[effects] pollution_filter_remaining: remaining native filtered-emission factor, 0..1 (0.775).
policy_duration_multiplier: original LOCK_AFTER_DAYS multiplier, 0.05..20 (1).
housing_height_threshold: residential research unlock height, 1..500 (33.8).
housing_quality_threshold: residential research unlock quality, 0.1..1 (0.899999).
Housing settings affect unlock thresholds, not the buildings' actual attributes.

Added research nodes, layout hooks and completion bonuses have been removed. Existing IDs/order/prerequisites are retained.
Legacy expansion options are ignored. Original/VFS files are composed through temporary data.
A nondefault pollution setting uses a guarded, version-specific private read; conflicts are refused.
Defaults retain original values. Keep backups of saves that previously used the expanded tree;
their migration requires separate validation. Game/save files are not directly overwritten.

GitHub: https://github.com/HONGxuanzhao/TESMIORESEARCH
