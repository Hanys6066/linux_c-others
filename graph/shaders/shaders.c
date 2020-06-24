#include "shaders.h"

/*_______________________Static Functions_______________*/
static char *loadShaderInMem(const char *RelPath);
static GLuint loadShader(const char *RelPath, unsigned int mode);
static char *loadShaderInMem(const char *RelPath);
static int InitShader(const char *path, GLuint *sh_id, GLuint mode);
static void cleanOneShader(GLuint program, GLuint shader);
/*___________________Global Functions____________________*/
int useShader(struct shPrg *Shader);
void cleanShader(struct shPrg *Shader);


/*___________________Functions Defintions_________________*/
static char *loadShaderInMem(const char *RelPath)
{
    int fd;
    char *absPath = RelPath_To_AbsPath(RelPath);
    char *code = (char *)NULL;
    printf("loadShaderInMem: %s\n", absPath);
    if(-1 ==(fd = open(absPath, 0, O_RDONLY))){
        printf("loadShaderInMem(): open() failed");
    }

    struct read_data data = read_file_data(fd, 1024, "r");
    if(data.isvalid == FALSE){
        printf("loadShaderInMem(): error during readinig");
        return((char *)NULL);
    }
    free(absPath);
    return data.data;
}

static GLuint loadShader(const char *RelPath, GLuint mode)
{
    GLuint sh_id;
    const char *source = loadShaderInMem(RelPath);
    char error[2000];
    sh_id = glCreateShader(mode);
    if(sh_id == 0){
        printf("loadShader(): glCreateShader() failed");
        return(0);
    }
    glShaderSource(sh_id, 1, &source, NULL);
    glCompileShader(sh_id);
    glGetShaderInfoLog(sh_id, SH_COMPLI_ERROR_MES_SIZE, NULL, error);
    printf("COMPILE ERROR: %s\n", error);
    return(sh_id);
}

static int InitShader(const char *path, GLuint *sh_id, GLuint mode)
{
    if(path == (char *)NULL){
        printf("useShader(): invalid Relvs");
        return(-1);
    }
    *sh_id = loadShader(path, GL_VERTEX_SHADER);
    if(*sh_id == 0){
        printf("InitShader(): loadShader() failed");
        return(-1);
    }
    return(0);
}


int useShader(struct shPrg *Shader)
{
    Shader->program = glCreateProgram();
    Boolean stat = FALSE;

    if(-1 != InitShader(Shader->Relvs, &(Shader->vs), GL_VERTEX_SHADER)){
        glAttachShader(Shader->program, Shader->vs);
        stat = TRUE;
    }
    if(-1 != InitShader(Shader->Relfs, &(Shader->fs), GL_FRAGMENT_SHADER)){
        glAttachShader(Shader->program, Shader->vs);
        stat = TRUE;
    }
    if(-1 != InitShader(Shader->Relgs, &(Shader->gs), GL_GEOMETRY_SHADER)){
        glAttachShader(Shader->program, Shader->vs);
        stat = TRUE;
    }
    if(-1 != InitShader(Shader->Reles, &(Shader->es), GL_TESS_EVALUATION_SHADER)){
        glAttachShader(Shader->program, Shader->es);
        stat = TRUE;
    }
    if(-1 != InitShader(Shader->Relcs, &(Shader->cs), GL_TESS_CONTROL_SHADER)){
        glAttachShader(Shader->program, Shader->cs);
        stat = TRUE;
    }
    if(stat == TRUE){/*at least one Shader was attach*/
        GLint status;
        glLinkProgram(Shader->program);
        glGetProgramiv(Shader->program, GL_LINK_STATUS, &status);
        if(status == GL_FALSE){/*link failed*/
            GLint logLen;   /*size of error message*/
            printf("useShader: Shader link failed():\n");
            glGetProgramiv(Shader->program, GL_INFO_LOG_LENGTH, &logLen);

            if(logLen > 0){
                char *err_msg = (char *)NULL;
                GLsizei numWrite;
                REALLOC(err_msg, logLen, 1, "useShader()", FALSE);
                glGetProgramInfoLog(Shader->program, logLen, &numWrite, err_msg);

                printf(err_msg);
                free(err_msg);
                return(-1);
            }else{
                printf("useShader(): failed but no error message is available\n");
            }
        }
        glUseProgram(Shader->program);
        return(0);
    }
    return(-1);
}

static void cleanOneShader(GLuint program, GLuint shader)
{
    if(shader != 0){
        glDetachShader(program, shader);
        glDeleteShader(shader);
    }
}

void cleanShader(struct shPrg *Shader)
{
    cleanOneShader(Shader->program, Shader->vs);
    cleanOneShader(Shader->program, Shader->fs);
    cleanOneShader(Shader->program, Shader->gs);
    cleanOneShader(Shader->program, Shader->es);
    cleanOneShader(Shader->program, Shader->cs);
    glDeleteProgram(Shader->program);
}
