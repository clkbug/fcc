TARGET := fcc

SRCS := \
	main.c \

OBJS := $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^