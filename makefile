all:
	mkdir data &> /dev/null
	gcc domainColoring.c -lm -lX11 -o domainColoring

clean:
	rm data/*ppm domainColoring
