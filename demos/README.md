# firmware 例程说明

- [firmware 例程说明](#firmware-例程说明)
	- [例程简介](#例程简介)
	- [基于本仓库的 MRS 例程建立新项目](#基于本仓库的-mrs-例程建立新项目)
		- [方法一 在例程旁边建立项目（推荐）](#方法一-在例程旁边建立项目推荐)
		- [方法二 使用 MRS 的导出功能](#方法二-使用-mrs-的导出功能)

## 例程简介

| 例程名                             | 开发环境         | 简介                 |
| ---------------------------------- | ---------------- | -------------------- |
| Camera                             | MRS              | OV2640摄像头例程     |
| CH32V307_RTT_VC_RC_by_es8388       | RT-Thread Studio | 语音识别例程         |
| GreedySnake_Net\GreedySnake_Client | MRS              | 贪吃蛇例程，客户端   |
| GreedySnake_Net\GreedySnake_Server | MRS              | 贪吃蛇例程，服务器端 |
| IIC_Sensor                         | MRS              | I2C 传感器例程       |
| Integrated_Test                    | MRS              | 出厂程序             |
| KEY_TEST                           | MRS              | 按键例程             |
| LCD_LVGL                           | MRS              | LVGL图形库例程       |
| Record_Play                        | MRS              | ES8388录放音例程     |
| RTT_RNG_Lottery                    | RT-Thread Studio | 随机抽奖例程         |
| SPI_Flash                          | MRS              | 片外FLASH例程        |
| TcpClient                          | MRS              | TCP 客户端例程       |
| USART/BLE_Serial                   | MRS              | CH9141蓝牙串口例程   |
| USART/Basic                        | MRS              | 串口轮询例程         |
| USART/UART_DMA                     | MRS              | 串口DMA例程          |
| USART/Wifi_ESP8266                 | MRS              | Wi-Fi 模组例程       |

## 基于本仓库的 MRS 例程建立新项目

本仓库中 MRS 例程的部分依赖文件放置于 'firmware/SRC' 文件夹中，单独复制例程到其它位置可能无法使用。  
可用以下方法基于本仓库例程建立新项目：（本仓库的结构与沁恒官方的 CH32V307 `EVT` 类似，下面的方法也适用于 `EVT` 中的例程）  

### 方法一 在例程旁边建立项目（推荐）

以此方法建立的项目不可移动或复制到其它位置使用。  
此处将示范以 `UART_Basic` 为基础，建立新项目：

1. 打开 `UART_Basic` 例程，在 `项目资源管理器` 中 右击 `UART_Basic`，选择 `复制`。（或者使用 `Ctrl C`)  
   ![复制](../doc/pic/copy.png)
2. 右击空白处，选择 `粘贴` （或者使用 `Ctrl V`)  
   ![粘贴](../doc/pic/paste.png)
3. 在弹出的对话框中修改项目名称，取消勾选 `使用缺省位置` 后，点击 `浏览`  
   ![改名](../doc/pic/changeName.png)
4. 到本项目的上一级目录中（此处是 `UART_Basic` 所在的 `USART` 文件夹中），新建文件夹并改名为项目名称  
   ![新建文件夹](../doc/pic/newFolder.png)
5. 选择新文件夹，返回对话框，点击 `复制`，就完成了。  
   ![完成](../doc/pic/complete.png)
6. 新项目编译通过  
   ![编译通过](../doc/pic/MRS_buildSuccess.png)

### 方法二 使用 MRS 的导出功能

以此方法导出的项目与原项目同名，可以移动或复制到其它位置使用。  
此处示范导出 `UART_Basic`：

1. 打开 `UART_Basic` 例程，在 `项目资源管理器` 中 右击 `UART_Basic`，选择 `导出`。  
   ![导出](../doc/pic/MRS_export.png)
2. 在弹出的对话框中选择 `常规` 分组中的 `文件系统`，点击 下一步  
   ![导出选项](../doc/pic/MRS_exportSelection.png)
3. 选择 `UART_Basic`，在 `选项` 中勾选 `创建文件的目录结构` 和 `解析并导出已连接资源`；再点击 `浏览` ，选择项目导出位置  
   ![导出设置](../doc/pic/MRS_exportConfig.png)
4. 点击 `完成`，即可完成项目导出。导出的项目包含所有的依赖文件。  
   ![导出的项目](../doc/pic/MRS_exported.png)
5. 导出的项目没有项目入口，可以通过 MRS 的 `加载` 功能打开（`文件` -> `加载`）。注意，加载的项目会覆盖 `项目资源管理器` 中的同名项目。  
   ![文件->加载](../doc/pic/MRS_load.png.png)
6. 在弹出的对话框中选择 `工程`，点击 `浏览`  
   ![加载对话框](../doc/pic/MRS_loadSelection.png)
7. 到导出的项目文件夹中，选择根目录下的 `.projuct` 文件，点击 `打开`  
   ![加载.projuct](../doc/pic/MRS_loadProject.png)
8. 返回对话框，点击 `确定`。如提示覆盖相同名称的工程，可选 `是`  
   ![覆盖](../doc/pic/MRS_loadOverlay.png)
9. 项目已导入，但一些文件夹仍处于未链接状态（带感叹号`!`）。需要将它们从项目中删除  
   ![删除链接文件夹](../doc/pic/MRS_delectUnlinked.png)
10. 此时编译，项目下会多出一些文件夹，且编译可以通过  
   ![编译通过](../doc/pic/MRS_exportSuccess.png)
