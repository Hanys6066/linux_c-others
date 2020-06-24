#ifndef SHADERS_H
#define SHADERS_H

#include <stdio.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include "../file_operations/my_file.h"

#define SH_COMPLI_ERROR_MES_SIZE    2000

struct shPrg{/*Shader program structure*/
        char            *Relvs;     /*Relative path to vertex Shader*/
        char            *Relfs;     /*Realative path to fragment Shader*/
        char            *Relgs;     /*Relative path to geometry Shader*/
        char            *Reles;     /*Relative path to tess evaluation shader*/
        char            *Relcs;     /*Relative path to tess control shader*/
        GLuint          vs;         /*vertex shader*/
        GLuint          fs;         /*fragment shader*/
        GLuint          gs;         /*geometry shader*/
        GLuint          es;         /*tess evaluation shader*/
        GLuint          cs;         /*tess control shader*/
        GLuint          program;    /*program*/
};

extern int useShader(struct shPrg *Shader);
/*Function initialize and run vertex shader and fragment shader,
->require valid Relative path of vertex Shader(Relvs) and Relative path of
fragment shader(Relfs) defined in shPrd structure*/
extern void cleanShader(struct shPrg *Shader);
/*ckean function for given Shader structure. function realise sources
used by Shaders*/

#endif
