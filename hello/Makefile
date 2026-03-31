FI=$(wildcard *.c )
PRO=$(patsubst %.c,%.o,$(FI))
TAR=hhh
DO= gcc

$(TAR): $(PRO)
	$(DO) $(PRO) -o $(TAR)

%.o: %.c
	$(DO) -c $< -o $@

clean:
	rm -i $(TAR) $(PRO)
