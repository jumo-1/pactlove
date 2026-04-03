FI=$(wildcard *.c )
PRO=$(patsubst %.c,%.o,$(FI))
TAR=hhh
DO= gcc

$(TAR): $(PRO)
	$(DO) -g $(PRO) -o $(TAR)

%.o: %.c
	$(DO) -g -c $< -o $@

clean:
	rm -i $(TAR) $(PRO)
