# ESP32-ding BLEcopy
某些蓝牙设备，比如某ding的卡机，他的蓝牙raw数据30秒变一次，这个项目可以远程复制他，（不需要自己有服务器）
# 实现的功能
用两个esp32   我这里用的esp32c3supermini，一个远程捕获蓝牙的raw数据   另一个在任何地方通电联网就能模拟出来（可以用 手机热点和otg）
# 准备工作
两个esp32 | Arduino IDE esp32开发环境 | [leancloud](https://console.leancloud.cn/)账号 | [nRF Connect app](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-mobile)

在leancloud创建一个应用 在其中的数据存储 结构化数据中 ，创建class  名称为AdvertisingData，在里面添加两列 为mac和data，设置 应用凭证 里面有appid appkey api![image](https://github.com/user-attachments/assets/9331a25b-d883-44da-9b05-3c7988fd340b)

nRF Connect 获取目标mac 和rawdata 备用

# 发送端配置   我的开发板库版本为3.0.7

Arduino IDE 把BLEcopy的代码粘贴进去，你需要修改appid appkey MAC和 第220行的apiurl 修改好烧录(可能会提示固件太大，工具partititon scheme选择NO OTA 2M APP/2M SPIFFS)重启后观察串口监视器，如果有输出则正常，三十秒后他会释放一个wifi，
连接后进入192.168.4.1配网，配网成功后串口监视器就会输出你要捕获的蓝牙raw信息，输出http响应码之后就可以进leancloud 看你创建的class有没有和nRF Connect中一样的data和raw信息 至此发送端完成。



# 接收端配置 接收端的esp32的开发板库版本必须为2.0.6！！！！！

把BLEserver的 代码复制进ide 你需要修改appid appkey api服务器地址 mac地址  
替换完成后烧录到开发板，观察串口监视器，配网之后 有输出raw信息，并且在nRF Connect app中是否扫描到开发板的蓝牙信号，接收端配置结束。

# 注意事项
接收端开发板库版本必须为2.0.6 ，esp32supermini比硬币还小，并且很便宜，天猫天降福利红包，5.8就能买一个
如果用的是esp32c3supermini 或者简约版 需要点击工具 USB CDC On Boot选择Enabled ,Flash mode 选择DIO

可以用手动修改leancloud上最新蓝牙raw的最后一位，再观察接收端串口监视器，和nRF Connect app扫描到的复制品蓝牙raw信息，是否实时更新来验证接收端的功能

如果安卓root 可以不用接收端直接[模拟蓝牙设备](https://github.com/Xposed-Modules-Repo/com.ztc1997.mockbluetoothdevice),发送端将完美代替蓝牙通告监听器

可以有多个接收端 leancloud每天api 3w次内免费 1GB存储内免费，实测一个发送端一个接收端24小时运行api次数为1.1w 每天占用1mb空间，
代码很臃肿但是实现了我的功能我也懒得改了，自用稳定了一周，没出现死机，蓝牙更新 模拟失败的情况

leancloud必须为中国大陆，国际版会403拒绝，暂时不会修复

# 鸣谢
[dingBle](https://github.com/LuisRain/dingBle/tree/858e01fe21fb3730ece37afea7de733591ba381d)

[com.ztc1997.mockbluetoothdevice](https://github.com/Xposed-Modules-Repo/com.ztc1997.mockbluetoothdevice)
# 仅用于蓝牙设备的开发与调试，请勿用于签到打卡等违法违纪用途。
