# ESP32-S3 N16R8 离线翻译机固件（MicroPython + ulab）

用 GitHub Actions 自动编译 **MicroPython v1.29.0** 固件，目标芯片为
**ESP32-S3-WROOM-1 N16R8**（16 MB Quad Flash + 8 MB Octal PSRAM）。

固件内置：

| 组件 | 版本 | 说明 |
|---|---|---|
| MicroPython | v1.29.0 | 正式版（GitHub tag；`post2` 是 PyPI 后缀） |
| ulab | 6.12.1 | numpy 风格数组/数值计算（tag 无 `v` 前缀） |
| `translate` 模块 | 内置 | **离线** 中英互译，二分查找，无网络依赖 |

---

## 仓库结构

```
.
├── .github/workflows/build.yml          # GitHub Actions 编译流水线
├── boards/
│   └── ESP32_GENERIC_S3_N16R8/
│       ├── mpconfigboard.h              # 板子标识
│       ├── mpconfigboard.cmake          # CMake 板子配置（1.29 必需）
│       ├── manifest.py                  # frozen manifest + C 模块声明
│       ├── sdkconfig                    # 16MB Flash + 16MB 分区表覆盖
│       └── partitions.csv               # 16MB 分区表（6MB app + 10MB VFS）
├── modules/
│   └── translate/
│       ├── micropython.cmake            # CMake 集成
│       ├── translate.c                  # C 模块实现
│       └── dict_data.h                 # 自动生成的词典数据（勿手改）
├── tools/
│   └── build_dict.py                    # CSV → dict_data.h 生成器
├── dict/
│   └── sample.csv                       # 示例中英词典（62 条）
└── README.md
```

---

## 快速开始

### 1. 推到 GitHub

```bash
git init
git add .
git commit -m "init: esp32s3 translate firmware"
git branch -M main
git remote add origin https://github.com/<your-name>/<your-repo>.git
git push -u origin main
```

Push 后 Actions 自动触发，约 8–15 分钟后在
**Actions → 最新 run → Artifacts** 下载，里面有 4 个文件：

| 文件 | 烧录地址 | 说明 |
|---|---|---|
| `bootloader.bin` | `0x0` | 二级引导 |
| `partition-table.bin` | `0x8000` | 分区表 |
| **`micropython.bin`** | **`0x10000`** | **MicroPython 应用本体**（含 ulab + translate） |
| `firmware.bin` | `0x0` | 合并镜像（上面三个合一，二选一即可） |

> **别搞混**：`firmware.bin` 是合并镜像（从 0x0 开始），`micropython.bin` 才是单独的应用。
> 分开烧三个文件时，应用必须用 `micropython.bin` → `0x10000`。

### 2. 烧录（esptool）

**安装 esptool：**

```bash
pip install esptool
```

**进入下载模式**（每次全新烧录前）：
按住 **BOOT** 键不放 → 按一下 **RST/EN** 键 → 松开 **BOOT** 键。
（部分开发板自动进入下载模式，无需手动操作。）

**端口确认：**
- Linux：`/dev/ttyUSB0` 或 `/dev/ttyACM0`
- macOS：`/dev/cu.usbserial-xxxx`
- Windows：`COM3`、`COM4` 等（设备管理器查看）

**首次烧录（擦除 + 三件套）：**

```bash
# 1. 擦除整片 Flash（首次烧录必做，清除旧分区表）
esptool.py --chip esp32s3 -p /dev/ttyUSB0 erase_flash

# 2. 烧录 bootloader + 分区表 + 应用（注意：应用是 micropython.bin，不是 firmware.bin）
esptool.py --chip esp32s3 -p /dev/ttyUSB0 \
  --baud 460800 \
  write_flash \
  --flash_size detect \
  0x0       bootloader.bin \
  0x8000    partition-table.bin \
  0x10000   micropython.bin
```

> **懒人版**：如果不想分三个文件，直接烧合并镜像：
> ```bash
> esptool.py --chip esp32s3 -p /dev/ttyUSB0 erase_flash
> esptool.py --chip esp32s3 -p /dev/ttyUSB0 write_flash 0x0 firmware.bin
> ```

**后续更新固件**（只刷应用，保留文件系统）：

```bash
esptool.py --chip esp32s3 -p /dev/ttyUSB0 \
  --baud 460800 \
  write_flash 0x10000 micropython.bin
```

**验证烧录：**

```bash
# 回读校验
esptool.py --chip esp32s3 -p /dev/ttyUSB0 verify_flash 0x10000 micropython.bin
```

> 如果 `--baud 460800` 报错或不稳定，降到 `115200`。

### 3. 验证 REPL

```python
import translate
translate.lookup("hello")     # '你好'
translate.lookup("世界")       # 'world'
translate.en2zh("computer")   # '电脑'
translate.zh2en("谢谢")        # 'thank you'
translate.lookup("xyz")       # None

import ulab.numpy as np
print(np.arange(5))
```

---

## 扩展词典

示例词典只有 62 条。实际使用请替换 `dict/sample.csv`：

```csv
en,zh
hello,你好
world,世界
...
```

**重新生成 `dict_data.h`：**

```bash
python3 tools/build_dict.py dict/sample.csv modules/translate/dict_data.h
```

CSV 规则：
- 第一行是表头（`en,zh`），会被自动跳过；
- 每行两列：英文词、中文释义，逗号分隔；
- 英文查找大小写不敏感；
- 脚本自动去重、双向建表（英→中 + 中→英）、排序；
- 词条无上限，全编进固件 Flash。1 万条约占 300–500 KB，完全放得下。

> GitHub Actions 每次构建都会自动重新跑 `build_dict.py`，所以你只需改 CSV 并 push，
> 不用手动提交 `dict_data.h`。

### 词典来源建议

参考你提到的项目：
- **mdict-utils / readmdict**：把 .mdx 词典转成纯文本/CSV，再喂给 `build_dict.py`；
- **arkhipenko/Dictionary**：B-tree + CRC32 思路，本模块用了更简单的二分查找（词典在编译期已排序，运行期无需建索引）。

---

## `translate` 模块 API

| 函数 | 说明 | 返回 |
|---|---|---|
| `translate.lookup(word)` | 自动判断中英方向，查不到返回 `None` | `str` 或 `None` |
| `translate.en2zh(word)` | 英译中 | `str` 或 `None` |
| `translate.zh2en(word)` | 中译英 | `str` 或 `None` |
| `translate.count()` | 词典英文词条数 | `int` |

---

## 关键配置说明

### 硬件参数（sdkconfig）

- Flash：16 MB QIO @ 80 MHz
- PSRAM：8 MB **Octal (OPI)** @ 80 MHz（N16R8 核心特征）
- 缓存：I-cache 32 KB + D-cache 32 KB，64-byte line

### 分区表（partitions.csv）

| 分区 | 偏移 | 大小 | 用途 |
|---|---|---|---|
| nvs | 0x9000 | 24 KB | WiFi/配置 |
| phy_init | 0xF000 | 4 KB | RF 校准 |
| factory | 0x10000 | 6 MB | MicroPython + ulab + translate + 词典 |
| vfs | 0x610000 | ~10 MB | LittleFS 文件系统 |

### 版本对应关系

| 组件 | 版本 | 可改？ |
|---|---|---|
| MicroPython | v1.29.0 | 改 `build.yml` 中 `MPY_VERSION` |
| ulab | 6.12.1 | 改 `ULAB_VERSION`（注意无 v 前缀） |
| ESP-IDF | v5.4.1 | 改 `IDF_VERSION`（需与 MP 版本兼容） |

---

## 注意事项与排错

### 编译阶段（GitHub Actions）

| 现象 | 原因 | 解决 |
|---|---|---|
| `Checkout ulab` 失败，git exit 1 | tag 名写错 | ulab 的 tag **没有 v 前缀**，用 `6.12.1` 不是 `v6.12.1` |
| `Checkout MicroPython` 失败 | tag 名写错 | MP 的 tag **有 v 前缀**，用 `v1.29.0` |
| IDF 编译报 `ESP-IDF version mismatch` | IDF 与 MP 不兼容 | 改 `IDF_VERSION`：先试 `v5.3.1`，再试 `v5.5` |
| ulab 编译报头文件/API 错误 | ulab 与 MP 版本不匹配 | 改 `ULAB_VERSION`，参考 ulab 仓库 README 的兼容矩阵 |
| 编译中途 OOM killed | runner 内存不足 | 把 `make -j3` 改成 `make -j2` 或 `-j1` |
| `translate.c` 报 `MP_REGISTER_MODULE` 未定义 | 缺头文件 | 已 include `py/builtin.h`，检查是否被覆盖 |
| 构建产物没有 `firmware.bin` | 板子名或编译路径不对 | 确认 `BOARD` 与板子目录名一致，build 目录是 `build-<BOARD>` |

### 烧录阶段（esptool）

| 现象 | 原因 | 解决 |
|---|---|---|
| `Failed to connect to ESP32-S3` | 没进下载模式 | 按住 BOOT → 按 RST → 松 BOOT |
| `A fatal error occurred: Failed to connect` | 串口被占用 / 驱动不对 | 关闭串口监视器；Linux 装 `cp210x` 或 `ch341` 驱动 |
| 烧录后没输出 / 不断重启 | PSRAM 配置不对 | 确认 sdkconfig 里 `CONFIG_SPIRAM_MODE_OCT=y`（N16R8 必须是 Octal） |
| `Segment 0 ... overlaps bootloader stack` / `No bootable app partitions` | 把 `firmware.bin` 烧到了 0x10000 | 应用必须用 **`micropython.bin`** → `0x10000`；`firmware.bin` 是合并镜像只能烧到 `0x0` |
| 烧录后 REPL 乱码 | 波特率不对 | 用 `115200` 打开串口（不是 9600） |
| `translate` import 失败 | C 模块没编进固件 | 检查 Actions 日志里 `USER_C_MODULES` 是否包含 translate 路径 |

### 运行阶段（REPL）

```python
# 快速自检
import translate, ulab.numpy as np
print(translate.lookup("hello"))     # 应输出 '你好'
print(translate.count())             # 应输出词典词条数
print(np.__version__)                # ulab 版本
```

### 关键技术点备忘

1. **1.29 已全面 CMake 化**：ESP32 port 入口仍是 `make BOARD=...`，但内部走 cmake；
   板子目录里的 `sdkconfig` 会自动合并到 ESP-IDF 配置。
2. **C 模块注册**：`micropython.cmake` 里接口库名必须以 `usermod_` 开头；
   C 代码里用 `MP_REGISTER_MODULE(MP_QSTR_translate, ...)` 注册。
3. **词典不占 RAM**：`dict_data.h` 里的表是 `static const`，直接放 Flash，
   运行期只占栈上几个指针，不消耗堆内存。
4. **PSRAM 是 Octal**：N16R8 的 R8 表示 8MB **Octal** PSRAM，不是 Quad。
   配错会导致启动崩溃或 WiFi 初始化失败。
5. **词典可无限扩**：改 `dict/sample.csv` 后 push，Actions 自动重新生成 `dict_data.h`。
   10 万条约占 3–5 MB，6MB 的 factory 分区放得下。

---

## License

本仓库代码（translate 模块、build 脚本、CI）可自由使用。词典数据版权归原作者，
请确认你使用的 CSV 词典许可。
