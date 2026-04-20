RS_485_PORT_SRC = rs_485_port/ring_fifo.c\
					rs_485_port/rs485_port.c\


# Required include directories
RS_485_PORT_INC = rs_485_port

ALLCSRC += $(RS_485_PORT_SRC)
ALLINC += $(RS_485_PORT_INC)
