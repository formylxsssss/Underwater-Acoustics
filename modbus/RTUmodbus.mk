MODBUS_SRC =  modbus/RTUmodbus_slave.c \
			 modbus/RTUmodbus_CRC.c 

MODBUS_INC = modbus \


SRC_FILES += \
  $(MODBUS_SRC)	
INC_FOLDERS += \
  $(MODBUS_INC) 