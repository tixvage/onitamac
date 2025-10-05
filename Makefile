GCC := gcc
CLIENT_BIN = client
SERVER_BIN = server
FLAGS += -Wall -Wextra -ggdb
CLIENT_FLAGS += -lm -lraylib
SERVER_FLAGS += 

ifeq ($(OS),Windows_NT)
	OS = windows
else
	UNAME := $(shell uname -s)
	ifeq ($(UNAME),Darwin)
		OS = macos
	else ifeq ($(UNAME),Linux)
		OS = linux
	else
    	$(error OS not supported by this Makefile)
	endif
endif

ifeq ($(OS),windows)
	CLIENT_BIN := $(CLIENT_BIN).exe
	SERVER_BIN := $(SERVER_BIN).exe
	CLIENT_FLAGS += -D_WIN32 -lws2_32 -lwinmm -lpthread -lgdi32 -lopengl32
	SERVER_FLAGS += -D_WIN32 -lws2_32 -lwinmm
else ifeq ($(OS),macos)
else ifeq ($(OS),linux)
endif

.PHONY: all 
all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): server.c
	$(GCC) $^ $(FLAGS) -o $(SERVER_BIN) $(SERVER_FLAGS)

$(CLIENT_BIN): client.c
	$(GCC) $^ $(FLAGS) -o $(CLIENT_BIN) $(CLIENT_FLAGS)

.PHONY: clean
clean:
	@$(RM) $(SERVER_BIN)
	@$(RM) $(CLIENT_BIN)