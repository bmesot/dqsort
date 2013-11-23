TARGETS = test_dqsort

all: $(TARGETS)

test_dqsort: stack.o dqsort.o test_dqsort.o
	gcc -Wall -pedantic -std=gnu99 -o $@ -lm -lpthread $^

%.o: %.c
	gcc -Wall -pedantic -c -std=gnu99 -o $@ $<

clean:
	rm -f *.o *~
	rm -f $(TARGETS)

depend:
	gcc -MM *.h *.c >.depend

include .depend
