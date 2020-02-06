#include <stdio.h>
#include <stdlib.h>

#define NUM_HOURS 10

struct official{
    char    init_char;
    char        *name;
    char           *c;
};

struct mathematic{
    char        *name;
    double init_value;
    double 	 *p_d;
};


typedef enum{
    MATHEMATIC,
    OFFICIAL
}WORKER;


union worker_type{
	struct official       offic;
	struct mathematic      math;
};

struct Worker{
	union worker_type     worker;
	union worker_type (*work)(union worker_type type, size_t num_hours);
};


/*!!!!!!!!!!!union cannot be pass by reference as function parametr only by value(C89)!!!!!!*/


/*in caase when we want union, containing dynamic allocated array, we must dynamicly allocate new union,
which will be able store this array. Function allocate_worker() create new union whose size is */

union worker_type allocate_worker(size_t old_size, size_t addition_memory)
{
  /*union cannot be pass as function parametr by value neither cannot be return as pointer(can but it can be unpredictable), according to the C89*/
	union worker_type *new_union;
	if(NULL == (new_union = (union worker_type *)malloc(old_size + addition_memory))){
		printf("malloc() error\n");
		exit(-1);
	}
	
	return *new_union;
}

union worker_type work_official(union worker_type type, size_t num_hours)
{
    union worker_type new_union;
    
    /*copy union items*/
    char *name = type.offic.name;
    char init_char = type.offic.init_char;
    
    printf("init char: %d\n",(int)(type.offic.init_char));
    printf("name: %s\n", type.offic.name);
    
    /*creation of new union, which can store dynamic allocated array*/
	  new_union = allocate_worker(sizeof(type), sizeof(char) * num_hours);
    
    /*copy items to the new union*/
    new_union.offic.init_char = init_char;
    new_union.offic.name = name;
    
	if(NULL == (new_union.offic.c = (char *)malloc(num_hours * sizeof(int)))){
		printf("malloc error()\n");
		exit(-1);
	}

	size_t i;
	for(i = 0; i < num_hours; i++){
		new_union.offic.c[i] = (int)(new_union.offic.init_char + i);
	}
	return new_union;
}

union worker_type work_mathematic(union worker_type type, size_t num_hours){
    union worker_type new_union;
    
    /*mathematic write character per hour*/
    double init_value = type.math.init_value;
    char *name = type.offic.name;
    
	new_union = allocate_worker(sizeof(type), sizeof(double) * num_hours);
    new_union.math.init_value = init_value;
    new_union.math.name = name;
    
	if(NULL==(new_union.math.p_d = malloc(10 * sizeof(double)))){
		printf("malloc error()\n");
		exit(-1);
	}

	size_t i;
	for(i = 0; i < num_hours; i++){
		new_union.math.p_d[i] = (double)(i + new_union.math.init_value);
	}
	return new_union;

}


void init_worker(struct Worker *worker, WORKER type){
   
    
    if(type == OFFICIAL){
        worker->work = work_official;
        worker->worker.offic.init_char = 48;
    }else if(type == MATHEMATIC){
        worker->work  = work_mathematic;
        worker->worker.math.init_value = 10;
    }else{
        printf("init_worker(): wrong argument type\n");
        exit(-1);
    }
    
}


int main(int argc, char **argv){

	size_t i;
	struct Worker w_1, w_2;
    
    /*size of */
    printf("size of official by sizeof: %d\n", (int)sizeof(struct official));
    printf("size of oficial: %d\n", (int)(2 * sizeof(char *) + sizeof(char)));
    printf("size of mathematic by sizeof: %d\n", (int)sizeof(struct mathematic));
    printf("size of mathematic: %d\n",(int)(sizeof(char *) + 2 * sizeof(double)));
    printf("true size of worker: %d\n", (int) sizeof(w_1.worker));

    init_worker(&w_1, OFFICIAL);
    printf("init char: %d\n",(int) w_1.worker.offic.init_char);
    w_1.worker.offic.name = "Pepa";
    init_worker(&w_2, MATHEMATIC);
    w_2.worker.math.name = "Lukas";    
    
    
    printf("init done\n");
    
    w_1.worker = w_1.work(w_1.worker, NUM_HOURS);
    w_2.worker = w_2.work(w_2.worker, NUM_HOURS);
    
    for(i = 0; i < 10; i++){
        printf("s_1[%d] = %c\n", (int)i, w_1.worker.offic.c[i]);
        printf("s_2[%d] = %lf\n", (int)i, w_2.worker.math.p_d[i]);
    }
	return 0;
}
