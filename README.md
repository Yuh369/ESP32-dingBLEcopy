# ESP32-dingBLEcopy
  某些蓝牙设备，比如某ding的卡机，他的蓝牙raw数据30秒变一次，这个项目可以远程复制他，（不需要自己有服务器）
# 实现的功能
    用两个esp32   我这里用的esp32c3supermini，一个远程捕获蓝牙的raw数据   另一个在任何地方通电联网就能模拟出来（可以用 手机热点和otg）
# 准备工作
    两个esp32 | Arduino IDE esp32开发环境 | [leancloud](https://console.leancloud.cn/)账号 | [nRF Connect app](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-mobile)
# 操作
    在leancloud创建一个应用 在其中的数据存储 结构化数据中 ，创建class  名称为AdvertisingData，在里面添加两列 为mac和data设置 应用凭证 里面有appid appkey api![image](https://github.com/user-attachments/assets/9331a25b-d883-44da-9b05-3c7988fd340b)

    nRF Connect 获取目标mac 和rawdata 备用

发送端配置
    Arduino IDE 把BLEcopy的代码粘贴进去，你需要修改appid appkey MAC和 第220行的apiurl 修改好烧录 重启后观察串口监视器，如果有输出则正常，三十秒后他会释放一个wifi
连接后 进入192.168.4.1配网配网成功后串口监视器就会输出你要捕获的蓝牙raw信息，输出http响应码之后就可以进leancloud 看你创建的class有没有和nRF Connect中一样的data和raw信息 至此发送端完成。

接收端配置 接收端的esp32的开发板库版本必须为2.0.6！！！！！
    把BLEserver的 代码复制进ide 你需要修改appid appkey api服务器地址 95行 140行的mac地址  222行的初始化蓝牙raw英文字母全大写 
替换完成后烧录到开发板，观察串口监视器，配网之后 有输出raw信息则正常，接收端配置结束。
# 注意事项
接收端开发板库版本必须为2.0.6  
