SHELL = /bin/sh
.PHONY: clean
NAME  = example
CC = gcc
CFLAGS = -g
TARGETS = pokus_neural
OBJFILES = $(NAME).o graph.o my_error.o my_file.o shaders.o

GRAPH_C = graph_lib/graph.c
GRAPH_H = graph_lib/graph.h
MY_FILE_H = file_operations/my_file.h
MY_FILE_C = file_operations/my_file.c
MY_ERROR_H = my_error/my_error.h
MY_ERROR_C = my_error/my_error.c
SHADERS_H = shaders/shaders.h
SHADERS_C = shaders/shaders.c

$(NAME): $(OBJFILES)
	$(CC) $(CFLAGS) -o $@ $^ -lm -lGL -lglut -lGLEW

$(NAME).o: $(NAME).c $(GRAPH_H)
	$(CC) $(CFLAGS) $(NAME).c -c

graph.o: $(GRAPH_H) $(SHADERS_H)
	$(CC) $(CFLAGS) $(GRAPH_C) -c

my_error.o: $(MY_ERROR_H)
	$(CC) $(CFLAGS) $(MY_ERROR_C) -c

my_file.o: $(MY_FILE_H) $(MY_ERROR_H)
	$(CC) $(CFLAGS) $(MY_FILE_C) -c

shaders.o: $(SHADERS_H) $(MY_FILE)
	$(CC) $(CFLAGS) $(SHADERS_C) -c

clean:
	rm -f *.o
