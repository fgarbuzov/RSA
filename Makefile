CC = g++
CFLAGS = -Wall -O2

SRC_DIR = src

SRC = $(wildcard $(SRC_DIR)/*.cpp)

EXEC = rsa.out

all: $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(SRC) -o $@ -lgmpxx -lgmp

clean:
	rm -f $(EXEC)
