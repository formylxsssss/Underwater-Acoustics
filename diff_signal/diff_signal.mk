# List of all the board related files.
DIFF_SIGNAL_SRC = diff_signal/diff_signal.c\
				diff_signal/channel_change.c\
				diff_signal/uw_link_tx.c\
				diff_signal/uw_link_rx.c\

# Required include directories
DIFF_SIGNAL_INC = diff_signal

ALLCSRC += $(DIFF_SIGNAL_SRC)
ALLINC += $(DIFF_SIGNAL_INC)
