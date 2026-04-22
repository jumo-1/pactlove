FILE=$(wildcard *.c)
TARGET=isproducer_consumer
CC=gcc

PRO=$(FILE:%.c=%.o)
DEPS=$(PRO:%.o=%.d)
CFLAGS= -Iinclude -Wall -MMD -MP -g 
$(TARGET): $(PRO)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -lpthread

clean:
	rm -rf $(PRO) $(TARGET) $(DEPS)
-include $(DEPS)

