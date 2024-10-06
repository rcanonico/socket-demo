# Compiler paths and executables
CC       = gcc

# Options
CFLAGS   = -Wall
LDFLAGS  = -pthread

TARGETS = ClientTCP ServerTCP ServerTCPc ServerTCPmt ServerTCPmth ClientUDP ServerUDP

# Targets
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

ServerTCPmth: ServerTCPmth.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS) *~



