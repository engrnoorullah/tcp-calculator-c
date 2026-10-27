CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
.PHONY: all test clean
all: build/tcp_server build/tcp_client
build:
	mkdir -p build
build/tcp_server: src/server.c src/net.h | build
	$(CC) $(CFLAGS) src/server.c -o $@
build/tcp_client: src/client.c src/net.h | build
	$(CC) $(CFLAGS) src/client.c -o $@
test: all
	python3 tests/test_network.py
clean:
	rm -rf build
