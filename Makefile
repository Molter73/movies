# Compilador y opciones de compilación
CC = gcc
CFLAGS = -I$(CURDIR)/src -pthread

# Nombres de archivos y carpetas
TARGET = movies
SRC_DIR = $(CURDIR)/src
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(wildcard $(SRC_DIR)/*.h)

# Reglas
all: CFLAGS += -O2 -DNDEBUG
all: $(TARGET)

debug: CFLAGS += -g
debug: $(TARGET)

asan: CFLAGS += -g -fsanitize=address,undefined
asan: $(TARGET)

tsan: CFLAGS += -g -fsanitize=thread
tsan: $(TARGET)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(SRC_DIR)/*.o $(MODULES_DIR)/*.o $(TARGET)
	rm -rf $(CURDIR)/outputs/
