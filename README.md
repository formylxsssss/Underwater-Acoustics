# Underwater-Acoustics
Underwater Acoustics


### modbus 定义
    目前modbus中所有数据全部存入03线圈寄存器中，一共5个float数据，共20字节，占用10个寄存器，目前设备id全部为01（测试用）

###### 
| 寄存器号 | 数据类型  | 数据位数 | 
| ---- | ----| ----|
| 0x00 | 温度 | 高位 |
| 0x01 | 温度 | 低位 |
| 0x02 | 湿度 | 高位 |
| 0x03 | 湿度 | 低位 |
| 0x04 | x_data | 高位 |
| 0x05 | x_data | 低位 |
| 0x06 | y_data | 高位 |
| 0x07 | y_data | 低位 |
| 0x08 | z_data | 高位 |
| 0x09 | z_data | 低位 |
