#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <limits.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /*standard int types and also here is defined SIZE_MAX*/
/*Pro funkce getLong() a getInt()*/
#define ERN_NONNEG   0x100  /*Čísla větší či rova 0 */
#define ERN_GTZERO   0x80 /*Čísla větší než 0*/

/*Pro funke getInt() a getLong() and getStr_FromInt() a getStr_FromLong*/
#define ERN_ANY_BASE 0x40   /*může být použita jakákoliv soustava*/
#define ERN_BASE_8   0x20  /*integer value will be print as octal value*/
#define ERN_BASE_16  0x10 /*integer value will be print as hexadecimal number*/

#define ERN_EXP 0x8   /*for float or double-> means that number will be in shape of d.dd e^(ddd)*/
#define ERN_HEX 0x4   /*integer value will be print as hexadecimal number*/
#define ERN_OCT 0x2   /*integer value will be print as octal value*/

/*Pro funkci errExit()*/
#define ER_WEXIT     0x0/*Pro standartní ukončení pomocí exit()*/
#define ER_W_EXIT    0x1 /*Pro ukončení pomocí _exit()*/

typedef enum {FALSE, TRUE} Boolean;


extern void errMsg(const char *format,...);/*vypiše kód errno chyby(např. EINVAL,...) a zprávu pokud je nějaká zadána
do argumetu format, vše se vypisuje na stderr, funkce neukončuje main() ani jinou void vracející funkci*/

extern void gnFail(const char *fname, const char *message, const char *arg, const char *name);
/*funkce vypisuje chybovou hlášku na STDERR->ta je dána argumenty funkce
fname -> jméno funkce ze které je gnfail() voláno
message -> zpráva(např jak chybu odstranit)
arg-> další info k popisu chyby
name-> lokalita či typ chyby(jedná se o chybný argument, přetečení rozsahu, ...)*/



/*funkce ošetřující používání funkcí strtol, strtof, strtod, strtold
při jakémkoli nezdaru funkce vrací nulu(!!!Neindikuje nutně chybu) a je nastaveno errno na nenulovou hodnotu(Skutečný indikátor chyby),
je tedy potřeba při použití funkce zkontrolovat errno zda-li je nula a pokud ano
je potřeba zkontrolovat zda je errno nastavneno na nenulovou hodnotu->nula neimplikuje chybu->
nula je také správný výsledek

errno = 0;
xxx cislo = getXXX(....);
if(errno != 0){
  do_something();
}
*/
extern long getLong(const char *arg, int flags, const char *name);
extern int getInt(const char *arg, int flags, const char *name);
extern float getFloat(const char *arg, int flags, const char *name);
extern double getDouble(const char *arg, int flags, const char *name);
extern long double getLDouble(const char *arg, int flags, const char *name);

/*function for receive string from number:
-str: string which will be return, if some error occur than function return -1 and errno is set
-digit: number of digit which will be save in str
-num: number which we want convert to the string
-flag: two kind of flags :
    1.flags which affect style of string(how it will look like)
        ERN_EXP 0x8   for float or double-> means that number will be in shape of d.dd e^(ddd)
        ERN_HEX 0x4   integer value will be print as hexadecimal number
        ERN_OCT 0x2   integer value will be print as octal value
    2.flags which restrict interval of number:
     ERN_NONNEG   0x100  Čísla větší či rova 0
    ERN_GTZERO   0x80 Čísla větší než 0
*/
extern int getStr_FromInt(char **str, const int digit, const int num, int flags, const char *name);
extern int getStr_FromLong(char **str, const int digit, const long num, int flags, const char *name);
extern int getStr_FromFloat(char **str, const int digit, const float num, int flags, const char *name);
extern int getStr_FromDouble(char **str, const int digit, const double num, int flags, const char *name);

#ifdef __GNUC__ /*specifikuje že se jedná o GNU kompilátor a budou se používat
                  GNU předdefinované makra(__attribute__(noreturn))*/
  /*_____________________________Others_marcos______________________________*/
  /*set pointer to null, it can be also array of pointers*/
  #define POINTERS_TO_NULL(pointer, num)\
      {\
          size_t i;\
          for(i = 0; i < num; i++){\
              pointer[i] = (__typeof__(*pointer))NULL;\
          }\
      }

  /*_________________REALLOC_________________*/
  #if(defined(__FreeBSD__) && (__FreeBSD__ >= 11)) || (defined(BSD) &&(BSD >=2014))
  /*if there is FreeBSD at least on version 11.0 or OpenBSD on version 5.6 reallocarray will be used instead of realloc->("");
  because in case of failure reallocarray doesnt change content of original array and check overflow(etc Integer overflow detection
  TODO: implement this in erro.c and rewrite all realloc -> to the REALLOC in project
  !!!!!if input variable is pointer to pinter-> set_to_zero cannot be used instead setting pointer to NULL(use macro POINTERS_TO_NULL), it
  only set pointer to zero adrress-> so user has to set allocated pointers on null by himself(otherwise mamory leak can occur)*/
      #define __USE_GNU 1
      #define REALLOC(ptr, num_member, element_size, outer_func, set_to_zero)\
      {
          /*printf("%s:%d num of size: %lld\n",outer_func, num_member, (long long)element_size);*/\
      typeof(ptr) _p_tmp;\
      if(NULL == (_p_tmp = (typeof(ptr))reallocarray((void *)ptr, num_member, element_size))){\
          free(ptr);\
          errExit(ER_WEXIT, "%s: reallocarray() error");\
      }\
      ptr = _p_tmp;\
      if(set_to_zero == TRUE){\
          size_t i;\
          for(i = 0; i < num_member; i++){\
              ptr[i] = (typeof(*ptr))0;\
          }\
      }\
      }
  #else
  /*it has to be inside some function, because when this marco is used in one function multiple times,*/
  /*than error will arise, because of multiple declaration od _p_tmp,
    if realloc is used, check of overflow(etc Integer overflow detection) must be added->if new size is smaller than SIZE_MAX*/
      #define REALLOC(ptr, num_member, element_size, outer_func, set_to_zero)\
      if(num_member && element_size && (SIZE_MAX / num_member < element_size)){\
           if(_ERRNO_H == 1){\
              errno = ENOMEM;\
          }\
          errExit(ER_WEXIT, "%s: wrong size of new array\n", outer_func);\
      }else{\
          size_t alloc_size = num_member * element_size;\
          /*printf("realloc:%d num of size: %lld\n",num_member, (long long)alloc_size);*/\
          typeof(ptr) _p_tmp = (typeof(ptr))NULL;\
          if(NULL == (_p_tmp = (typeof(ptr))realloc(ptr, alloc_size))){\
              free(ptr);\
              errExit(ER_WEXIT,"%s: realloc() error", outer_func);\
          }\
          ptr = _p_tmp;\
          if(set_to_zero == TRUE){\
              size_t i;\
              for(i = 0; i < num_member; i++){\
                  ptr[i] = (typeof(ptr[0]))0;\
              }\
          }\
          }
  #endif

#define NORETURN __attribute__((noreturn)) /*GNU nebude vyvolávat varováníu funkcí používající exit() nebo abort()*/
#else
#define NORETURN
#endif

extern void errExit(int flag, const char *format,...);/*chová se stejně jako errMessage, navíc ukončuje proces
 pomocí volání exit() je-li specifikovan flag ER_WEXIT
 pomocí volání _exit() je-li specifikován flag ER_W_EXIT*/

extern void errExitErrnum(int errnum, const char *format,...) NORETURN;/*funkce nepoužívá errno ale jako parametr bere hodnotu chyby (integer)
využít v případech kdy nelze použít errno (např. u POSIX vláken)*/

extern void fatal(const char *format,...) NORETURN;/*pro vyhodnocení obecné chyby, kdy není k dispozici errno ani hodnota chyby(integer),
vypíše se pouze chybová hláška zadaná do parametru format, ukončení se provede pomocí exit()*/

extern void usageErr(const char *format,...) NORETURN;/*vytisne chlášku na stderr ve tvaru |"Usage error:" + format+"\n"| a provede volání fflush na stdout a stderr*/

#endif
