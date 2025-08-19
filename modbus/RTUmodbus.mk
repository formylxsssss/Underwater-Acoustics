MODBUS_SRC =  modbus/RTUmodbus_slave.c \
			        modbus/RTUmodbus_CRC.c 

MODBUS_INC = modbus 

ALLCSRC += $(MODBUS_SRC)
ALLINC += $(MODBUS_INC)
