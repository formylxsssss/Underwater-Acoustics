# List of all the board related files.
SENSOR_DEVICE_SRC = sensor_device/ADXL345.c\
					sensor_device/DHT22.c\
					sensor_device/eeprom.c\
					sensor_device/soft_timer.c\
					sensor_device/data_collect.c

# Required include directories
SENSOR_DEVICE_INC = sensor_device

ALLCSRC += $(SENSOR_DEVICE_SRC)
ALLINC += $(SENSOR_DEVICE_INC)
