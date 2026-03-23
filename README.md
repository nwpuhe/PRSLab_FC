
# 🛠️ Ubuntu + VS Code + PlatformIO 嵌入式开发环境搭建

本指南旨在帮助开发者在 Ubuntu 环境下，快速搭建基于 VS Code 和 PlatformIO (PIO) 的嵌入式开发环境，彻底解决 DAPLink 等硬件调试器的 USB 读写权限问题，并规范化工程的目录结构。

## 1. 基础环境安装

1. 安装 **Visual Studio Code**。
2. 在 VS Code 左侧扩展商店中，搜索并安装 **PlatformIO IDE** 插件。
3. 等待右下角提示 `PlatformIO Core has been successfully installed` 后，重启 VS Code。

---

## 2. 彻底解决 USB 烧录权限问题 (核心)

在 Linux 系统中，普通用户默认没有直接操作底层 USB 硬件设备（如 DAPLink, ST-Link）的权限。如果不进行提权配置，PIO 在调用 OpenOCD 烧录时会报错 `unable to open CMSIS-DAP device`。

请依次在终端执行以下命令，完成“一劳永逸”的免驱权限配置：

**第一步：下载并写入 PIO 官方 Udev 规则**
> 该规则包含了市面上绝大多数烧录器（包括各类魔改 DAPLink）的 PID/VID 授权信息，并屏蔽了 ModemManager 的干扰。
```bash
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/master/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
```

**第二步：将当前用户加入硬件特权组**
> 赋予当前用户操作串口和 USB 设备的权限。
```bash
sudo usermod -a -G dialout,plugdev $USER
```

**第三步：重载规则让系统底层生效**
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

**第四步：物理拔插与权限挂载**
1. **必须**将插在电脑上的 DAPLink **拔下并重新插上**，让系统对该设备应用全新规则。
2. **必须注销当前 Ubuntu 用户并重新登录**（或直接重启电脑）。因为用户组（Group）权限的变更只有在用户重新登录后才会真正生效。

---

## 3. 编译安装最新版 OpenOCD (支持最新 MCU)
```bash
sudo apt update
sudo apt install -y make libtool pkg-config autoconf automake texinfo libusb-1.0-0-dev

git clone --recursive https://github.com/openocd-org/openocd.git
cd openocd
./bootstrap
./configure --enable-cmsis-dap --enable-stlink --enable-jlink --enable-ftdi
make -j$(nproc)
sudo make install
```
在终端输入 openocd --version，确保版本号为最新（通常是 0.12.0+ 或 0.13.0-rc）。

---

## 4. 工程配置 (`platformio.ini`)

环境打通后，需要强制指定项目使用 DAPLink 进行烧录和调试。打开项目根目录下的 `platformio.ini` 文件，添加以下配置：

```ini
[env:你的开发板名称]
platform = ststm32
board = 你的开发板名称
framework = stm32cube

; 强制指定使用 DAPLink (CMSIS-DAP) 进行下载和调试
upload_protocol = cmsis-dap
debug_tool = cmsis-dap
```

---

## 5. PIO 工程目录规范

为了保证代码的可维护性，请严格按照以下职能划分存放代码：

* **`src/` (Source / 核心源代码)**：
  * **必须将 `main.c` 放在此目录下。**
  * 存放项目的核心业务逻辑、中断处理函数等。当你点击 Build 编译时，只有这里的代码会被打包进最终的 `firmware.elf` 固件中。
* **`include/` (头文件)**：
  * 存放自定义的 `.h` 头文件，PIO 会自动将其加入编译器的 Include Path。
* **`lib/` (私有库)**：
  * 存放可以在其他项目中复用的独立外设驱动模块（包含独立的 `.c` 和 `.h`）。
* **`test/` (单元测试)**：
  * **严禁将带有 `main()` 函数的产品代码放入此目录！**
  * 仅用于存放基于 Unity 框架的自动化测试脚本。在正常编译（Build/Upload）时，该目录下的文件会被编译器直接忽略。
* **`.pio/` (构建产物)**：
  * 自动生成的中间文件和最终固件目录，**请勿提交到 Git 仓库**。

---

## 6. 编译与烧录验证

代码编写完成后，连接好 DAPLink 与目标板：
1. 点击 VS Code 底部蓝色状态栏的 **`✓` (Build)** 图标进行编译。
2. 编译通过后，点击旁边的 **`→` (Upload)** 图标。
3. 如果终端最终输出绿色的 `[SUCCESS]`，硬件成功复位并运行，说明开发环境已完美搭建完毕！
