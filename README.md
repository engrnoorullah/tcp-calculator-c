# TCP calculator in C

A small POSIX socket client and server, based on my MSc network-programming math-server assignment at UET Peshawar. The server accepts a signed-integer arithmetic expression and returns its result. A child process handles each connection so that one incomplete request does not stop the listening process from serving another client.

This is a cleaned 2026 edition of historical coursework. The submitted files, including sequential, `fork()` and `select()` variants, are retained in `original/` as text. The current build implements the fork-based version. It is an educational programming sample, not a production calculator service.

## Build and use

Requirements: Linux or another POSIX environment with a C11 compiler and Make. Python 3 is needed for the tests. Tested with GCC on Linux. Windows requires a suitable POSIX environment, such as WSL.

```sh
make
./build/tcp_server 5000
```

In another terminal:

```sh
./build/tcp_client 127.0.0.1 5000 '12 + 30'
# OK 42
./build/tcp_client 127.0.0.1 5000 '-9 / 2'
# OK -4
./build/tcp_client 127.0.0.1 5000 '5 / 0'
# ERR division by zero
```

Stop the server with Ctrl-C. Run `make test` to check arithmetic, invalid requests, fragmented TCP input, concurrent clients, incomplete requests and CLI argument validation. Tests use loopback and an OS-assigned free port.

## Protocol

Each connection carries one ASCII request ending in a newline, followed by one newline-terminated response. Operands are signed 32-bit integers. Operators are `+`, `-`, `*` and `/`. Calculations use 64-bit integers; division truncates toward zero. Spaces are allowed. Requests are limited to 126 bytes before the newline. Responses begin with `OK` or `ERR`.

The client exits with status `0` for `OK`, `2` for a server-reported error and `1` for an input or transport failure. A connection has a five-second request deadline and a six-second process alarm covering blocked operations. TCP messages are framed by a newline; a single `recv()` is not assumed to contain the entire request.

The server binds to `127.0.0.1` by default. An optional second argument selects a bind IPv4 address. There is no authentication, TLS, global connection quota or service supervisor. Keep the demonstration on loopback or a controlled lab network. The alarm bounds individual connections; it is not a hard real-time guarantee.

## Provenance and changes

The originals came from `Noorullah_All9TasksComplete.zip`, Task 1, in my supplied Network Programming archive. Their headers identify Noor Ullah and the Electrical Engineering Department. The course's `unetprog.h` support header is not included in that final task archive. The maintained version uses standard POSIX headers and a small replacement helper instead.

Changes include explicit request framing, complete sends, checked parsing, rejection of division by zero and out-of-range operands, 64-bit arithmetic, cleanup of descriptors and child processes, local binding, timeouts, build instructions and integration tests. These changes were made in 2026; the repository does not imply that the original coursework already had all of them.

The coursework follows conventional UNIX socket-programming patterns taught in networking courses. No textbook code archive, books, lecture slides, private coursework data or precompiled executables are published here. Original file checksums are in `original/SHA256.json`.

Original coursework author: Noor Ullah. No redistribution licence has been selected for this repository.
