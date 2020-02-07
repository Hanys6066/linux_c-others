#include <stdlib.h>
#include <stdio.h>



#define PRINT_2D_D_ARRAY(array, d1, d2) {\
    size_t i, j;\
    for(i = 0; i < d1; i++){\
        for(j = 0; j < d2; j++){\
                printf("array[%lld][%lld]: %lf\n", i ,j, array[i][j]);\
        }\
    }\
}

#define SET_POINTER_ARRAY_TO_NULL(pointer, size) {\
    size_t i;\
    for(i = 0; i < size; i++){\
        pointer[i] = (__typeof__(*pointer))NULL;\
    }\
}

struct some_structure{
        int i;
        double **array;
};


/*if we want initailize array in some functon, we have to pass pointer on this array by reference(by pointer), because in case of passing by value,
 changes on passed vector will be discarted after function return*/
static int allocate_array2d(double ***array, size_t d1, size_t d2){
    
    size_t i, j;
    /*first dimension*/
    if(NULL == ((*array) = (double **)malloc(d1 * sizeof(double *)))){
        return(-1);
    }
    
    SET_POINTER_ARRAY_TO_NULL((*array), d1);
    printf("allocation done\n");
    /*second dimension*/
    for(i = 0; i < d1; i++){
        if(NULL == ((*array)[i] = (double *)malloc(d2 * sizeof(double)))){
            return(-1);
        }
        /*fill array with some values*/
        for(j = 0; j < d2; j++){
            (*array)[i][j] = j + i;
        }
    }
    
    return 0;
}


int main(int argc, char *argv)
{
    struct some_structure s;
    
    s.i = 2;
    s.array = (double **)NULL;
    
    if(-1 == allocate_array2d(&s.array, 10, 3)){
        printf("some error occur\n");
    }
    
    PRINT_2D_D_ARRAY(s.array, 10, 3);
    return 0;
}
