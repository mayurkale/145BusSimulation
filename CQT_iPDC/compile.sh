#!/bin/sh

# Compilation commands are: first run command "chmod +x compile.sh" then use -> 1) sh compile.sh, 2)  ./compile.sh

	gcc -c new_pmu_or_pdc.c -o new_pmu_or_pdc.o
	gcc -c parser.c -o parser.o
	gcc -c dds.c -o dds.o
	gcc -c connections.c -o connections.o
	gcc -c align_sort.c -o align_sort.o
	gcc -c dallocate.c -o dallocate.o
	gcc -c applications.c -o applications.o
	gcc -c apps.c -o apps.o
	gcc -g ipdc.c new_pmu_or_pdc.o parser.o dds.o connections.o align_sort.o applications.o apps.o dallocate.o -o iPDC -lrt -lm -lpthread
	
#	rm new_pmu_or_pdc.o parser.o dds.o connections.o align_sort.o dallocate.o applications.o apps.o iPDC
	
