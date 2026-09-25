# Builds the library binary; OpenSSL provides SHA-256, ECDSA and PBKDF2.

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
