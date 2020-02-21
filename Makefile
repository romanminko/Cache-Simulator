OBJS = main.o cache.o
READFILE = test.txt
VARIABLE=`cat $(READFILE)`


project : $(OBJS)
	gcc $(OBJS) -o cache
main.o : main.c
	gcc main.c -c
cache.o : cache.c
	gcc cache.c -c
clean:
	rm $(OBJS) cache
run : cache
	./cache test.txt
silent : cache
	gcc -DSILENT main.c cache.c -o cache
output : cache
	./cache test.txt > output.txt
