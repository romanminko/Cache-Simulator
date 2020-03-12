OBJS = main.o cache2.o
READFILE = test.txt
VARIABLE=`cat $(READFILE)`


project : $(OBJS)
	gcc $(OBJS) -o cache2
main.o : main.c
	gcc main.c -c
cache2.o : cache2.c
	gcc cache2.c -c
$(OBJS) : cache2.h
clean:
	rm $(OBJS) cache2
run : cache2
	./cache2 test.txt
silent : cache2
	gcc -DSILENT main.c cache2.c -o cache2
output : cache2
	./cache2 test.txt > output.txt
