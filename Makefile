pal.exe : image.c test.c mediancut.c
	gcc -g -o $@ $^

clean :
	rm pal.exe
