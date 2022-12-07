TARGET := fcc
CFLAGS := -Wall -Werror -Wpedantic

SRCS := \
	main.c \

OBJS := $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: test clean
test: $(TARGET)
	./test/test

clean:
	rm -f $(TARGET) $(OBJS)