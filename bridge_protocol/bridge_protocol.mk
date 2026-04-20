BRIDGE_PROTOCOL_SRC = bridge_protocol/bridge_app.c\
					bridge_protocol/bridge_proto.c\
					bridge_protocol/bridge_user_hooks.c\
					bridge_protocol/uw_link_proto.c\

# Required include directories
BRIDGE_PROTOCOL_INC = bridge_protocol

ALLCSRC += $(BRIDGE_PROTOCOL_SRC)
ALLINC += $(BRIDGE_PROTOCOL_INC)
