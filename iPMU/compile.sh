#!/bin/sh

   # Compilation commands are: first run command "chmod +x compile.sh" then use -> 1) sh compile.sh, 2)  ./compile.sh
#   gcc -c function.c -o function.o

#  gcc -c callbacks.c -o callbacks.o `pkg-config --cflags --cflags gtk+-2.0``pkg-config --cflags --libs gtk+-2.0` -lpthread

#   gcc -c ServerFunction.c -o ServerFunction.o `pkg-config --cflags --cflags gtk+-2.0``pkg-config --cflags --libs gtk+-2.0` -lpthread

#   gcc -c CfgFunction.c -o CfgFunction.o `pkg-config --cflags --cflags gtk+-2.0``pkg-config --cflags --libs gtk+-2.0`

#   gcc -c CfgInstallation.c -o CfgInstallation.o `pkg-config --cflags --cflags gtk+-2.0``pkg-config --cflags --libs gtk+-2.0`

#   gcc -Wall -g pmu.c callbacks.o ServerFunction.o function.o CfgFunction.o CfgInstallation.o -o pmu `pkg-config --cflags --cflags gtk+-2.0``pkg-config --cflags --libs gtk+-2.0` -lpthread

	gcc -c function.c -o function.o
	gcc -c connection.c -o connection.o
	gcc -Wall -g iPMU.c function.o connection.o -o iPMU -lrt -lm -lpthread
