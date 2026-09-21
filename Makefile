# Simple Makefile - no fancy pattern rules, just enough to build one binary.
#
# CC       : which compiler to use
# CFLAGS   : -Wall (show all warnings) -Wextra (a few more) -std=c11 (language version)
# LDLIBS   : link against OpenSSL's crypto library (SHA-256 + ECDSA live here)

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDLIBS = -lssl -lcrypto

SRC = src/main.c src/blockchain.c src/crypto.c src/registry.c
BIN = library

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC) $(LDLIBS)

clean:
	rm -f $(BIN)

.PHONY: clean
