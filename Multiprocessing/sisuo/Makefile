FILE=$(wildcard *.c)
TARGET=processing
CC=gcc

PRO=$(FILE:%.c=%.o)
DEPS=$(PRO:%.o=%.d)
CFLAGS= -Iinclude -Wall -MMD -MP -g
$(TARGET): $(PRO)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(PRO) $(TARGET) $(DEPS)
-include $(DEPS)

