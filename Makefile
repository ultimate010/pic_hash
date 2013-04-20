all:processPic.c
	mpicc processPic.c -ljpeg -lpng
run:all
	mpirun -n 3 a.out
clean:
	rm a.out tags
