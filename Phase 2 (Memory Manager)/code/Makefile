build:
	gcc process_generator.c -o process_generator.out `pkg-config --cflags --libs gtk+-3.0 cairo` -lm
	gcc clk.c -o clk.out `pkg-config --cflags --libs cairo` -lm
	gcc scheduler.c -o scheduler.out `pkg-config --cflags --libs cairo` -lm
	gcc process.c -o process.out `pkg-config --cflags --libs cairo` -lm
	gcc test_generator.c -o test_generator.out `pkg-config --cflags --libs cairo`

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out
