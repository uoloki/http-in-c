CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -D__USE_MINGW_ANSI_STDIO=1
SRCS    = src/main.c src/server.c
LIBS    = -lws2_32
BIN     = bin/rest.exe
CLEAN   = rmdir /S /Q bin 2>NUL & del /Q src\*.o 2>NUL

all: $(BIN)

$(BIN): $(SRCS)
	@mkdir bin
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@$(CLEAN)
.PHONY: all clean
