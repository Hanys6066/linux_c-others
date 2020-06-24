#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "my_error.h"
#include "ename.c.inc" /*Definuje promenou ename(ta uchovává názvy errno hodnot, hodnota dané chyby koresponduje s pozicí v poli)
a MAX_ENAME */

#ifdef __GNUC__
/*GCC extension (used macros)*/
__attribute__ ((__noreturn__))
#define max(a,b) \
  ({ __auto_type _a = (a); \
     __auto_type _b = (b); \
     _a > _b ? _a : _b; })
#endif

/*----------------------------------statické funkce---------------------------------------*/
static void terminate(Boolean use_Exit);
static void outputError(Boolean useErr,int err,Boolean flushStdout,const char *format,va_list ap);
static long getNum(const char *fname, const char *arg, int flags, const char *name);

/*------------------------------------globální funkce-------------------------------------*/
void gnFail(const char *fname, const char *message, const char *arg, const char *name);
void errMsg(const char *format,...);
void errExit(int flag, const char *format,...);
void fatal(const char *format,...);
void usageErr(const char *format,...);
int getStr_FromFloat(char **str, const int digit, const float num, int flags, const char *name);
int getStr_FromDouble(char **str, const int digit, const double num, int flags, const char *name);
int getStr_FromLong(char **str, const int digit, const long num, int flags, const char *name);
int getStr_FromInt(char **str, const int digit,const int num, int flags, const char *name);
long getLong(const char *arg, int flags, const char *name);
int getInt(const char *arg, int flags, const char *name);
float getFloat(const char *arg, int flags, const char *name);
double getDouble(const char *arg, int flags, const char *name);
long double getLDouble(const char *arg, int flags, const char *name);
/*----------------------------------------------------------------------------------------*/

static void terminate(Boolean use_Exit)
{
  /*funkce ukončující proces, pokud naastave chyba jádra-> obsah systémové proměnné EF_DUMPCORE je nějaká chyba->
   proces se ukončí abort()->abnormální ukončení procesu(nejprve odblokuje SIGABRT signál
   a následně tento signál pošle pomocí funkce raise() procesu v
   nemž byla abort() volána, toto vede k abnormálnímu ukončení, pokud není na signál SIGABRT nastaven
   signal handler využívjící longjmp())
   -Pokud chyba jádra nenastane funkce proces ukončí buď pomocí exit() nebo funkce _exit(), při ukončení pomocí _exit()
   nedojde k provedení "odklízecí" funkce specifikované v argumentu atexit(), je-li nějaká nastavena a nedojde k probvedení
   flush() na stdio bufrech*/
  char *s;

  s = getenv("EF_DUMPCORE");/*funkce getenv() slží k získání informací v podobě
  stringu o systémových proměnných-> v tomto případě o proměnné uchovávající kód chyby jádra*/

  if(s != NULL && *s != '\0'){
    abort();
  }else if(use_Exit){
    exit(EXIT_FAILURE);
  }else{
    _exit(EXIT_FAILURE);
  }
}


static void outputError(Boolean useErr,       /*určuje zdaa se použije errno(TRUE) nebo ne(FALSE)*/
                        int err,              /*používáme-li errno pak, do tohoto parametru vložíme
                                                errno(při volání funkce)*/
                        Boolean flushStdout,  /*specifikuje zda se provede volání fflush(stdut)*/
                        const char *format,   /*definuje počet a typ konverzních specifikátorů*/
                        va_list ap            /*proměnná typu va_list(u funkc s proměnným počtem argumentů),
                                                nutná pro vypsání zprávy*/
                      )
{
  #define BUF_SIZE 500

  char buf[BUF_SIZE];
  char userMsg[BUF_SIZE];
  char errText[BUF_SIZE];

  vsnprintf(userMsg, BUF_SIZE, format, ap);/*funkce podobná snprintf(), ale místo proměnného počtu operátorů
                                            používá va_list(do userMsg uloží textový řetězec daný ap a format
                                            o max velikosti BUF_SIZE)*/

  if(useErr){
    snprintf(errText, BUF_SIZE, "[%s %s]",/*do errText vypíše dva textové řetězce o celkové max. velikosti BUF_SIZE*/
             (err > 0 && err <= MAX_ENAME) ? ename[err] : "?UNKNOWN?",/*pokud je hodnota err v daném rozsahu je do prvního řetezce vypsáno ename[err],
                                                                        což je příslušené označení chyby errno(např. EINVAL), jinak je vytištěno ?UNKNOWN?*/
             strerror(err));                                          /*jako druhý řetezec je popis chyby daný funkcí strerror()*/

  }else{
    snprintf(errText, BUF_SIZE, ":");
  }

  snprintf(buf, BUF_SIZE, "ERROR: %s %s\n", errText, userMsg);

  if(flushStdout){
    fflush(stdout);   /*vykreslení jakéhololiv čekajícího stdout*/
  }
  fputs(buf, stderr);
  fflush(stderr);
}


static long getNum(const char *fname, const char *arg, int flags, const char *name)
{
  long res;
  char *endptr;
  int base;

  if(arg == NULL || *arg == '\0'){
    gnFail(fname,"empty or null string", arg, name);
    errno = EINVAL;
    return (long)0;
  }

  base = (flags & ERN_ANY_BASE) ? 0 :(flags & ERN_BASE_8) ? 8 : (flags & ERN_BASE_16) ? 16 : 10;

  errno = 0;
  res = strtol(arg, &endptr, base);
  if(errno != 0){ /*kontrola správnosti funkce pomocí errno(obecně se to takhle nedoporučuje,
                  ale strtol nevrací -1 v případě chyby !!, vrací LONG_MAX nebo LONG_MIN
                při přetečení nebo podtečení a nebo vrací 0 v případě, že nedošlo k žádné konverzi->žádné Čísla
              nebyla rozpoznána)*/
    gnFail(fname, "strtol() failed", (char *)NULL, name);
    return (long)0;
  }

  if(*endptr != '\0'){
    gnFail(fname, "nenumericky znak", arg, name);
    errno = EINVAL;
    return (long)0;
  }

  if((flags & ERN_NONNEG) && res < 0){
    gnFail(fname, "akceprovatelna jsou pouze kladna cisla", arg, name);
    errno = ERANGE;
    return (long)0;
  }

  if((flags & ERN_GTZERO) && res <= 0){
    gnFail(fname, "akceprovatelna jsou pouze nezaporna cisla", arg, name);
    errno = ERANGE;
    return (long)0;
  }

  return res;
}


void gnFail(const char *fname, /*name function, in wchich gnFail is call*/
            const char *message,/*message about error->why argument is wrong*/
            const char *arg,/*argument of function which is observe by gnFail()*/
            const char *name/*name of outer function*/
          )
{

  dprintf(STDERR_FILENO, "%s error ", fname);
  if(fname != NULL){
    dprintf(STDERR_FILENO, "in %s ", message);
  }

  dprintf(STDERR_FILENO, ": %s\n", name);
  if(arg != NULL && *arg != '\0'){
    dprintf(STDERR_FILENO, "%s\n", arg);
  }
}


void errMsg(const char *format,...)
{
  va_list arg;
  int errsv;         /*pro uložení errno*/

  errsv = errno;     /*uložení errno, */

  va_start(arg, format);
  outputError(TRUE, errno, TRUE, format, arg);
  va_end(arg);
  printf("\n");
  errno = errsv;
}


void errExit(int flag,
            const char *format,
            ...
            )
{
  va_list arg;

  va_start(arg, format);
  outputError(TRUE, errno, TRUE, format, arg);
  va_end(arg);

  if(flag == ER_WEXIT){
    terminate(TRUE);
  }else if(flag == ER_W_EXIT){
    terminate(FALSE);
  }else{
    dprintf(STDERR_FILENO,"errExit() failed, chyba argumentu flag\n");
  }
}


void errExitErrnum(int errnum,
                   const char *format,
                   ...
                   )
{
  va_list arg;

  va_start(arg, format);
  outputError(TRUE, errnum, TRUE, format, arg);
  va_end(arg);

  terminate(TRUE);
}


void fatal(const char *format,...)
{
  va_list arg;

  va_start(arg, format);
  outputError(FALSE,0,TRUE, format, arg);
  va_end(arg);

  terminate(TRUE);
}


void usageErr(const char *format,...)
{
  va_list arg;

  dprintf(STDERR_FILENO, "Usage: ");
  va_start(arg, format);
  vdprintf(STDERR_FILENO, format, arg);
  va_end(arg);

  fflush(stderr);
  exit(EXIT_FAILURE);
}

long getLong(const char *arg, int flags, const char *name)
{
  long res;
  errno = 0;
  res = getNum("getLong", arg, flags, name);
  printf("getLong: %ld\n", res);
  if(errno != 0){
      return (long)0;
  }
  return res;
}


int getInt(const char *arg, int flags, const char *name)
{
  long res;
  errno = 0;
  res = getNum("getInt", arg, flags, name);/*ve funkci je errno = 0*/

  if(errno != 0){
      return (int)0;
  }

  if(res > INT_MAX || res < INT_MIN){
    gnFail("getInt","integer out of range", arg, name);
    if(errno == 0){
      errno = ERANGE;
    }
    return (int)0;
  }

  return (int)res;
}

float getFloat(const char *arg, int flags, const char *name)
{
  float res;
  char *endptr;

  errno = 0;
  res = strtof(arg, &endptr);

  if(errno != 0){
    gnFail("getFloat", "strtof() failed", (char *)NULL, name);
    return (float)0;
  }

  if(*endptr != '\0'){
    gnFail("getFloat", "nenumericky znak", arg, name);
    errno = EINVAL;
    return (float)0;
  }

  if((flags & ERN_NONNEG) && res < 0){
    gnFail("getFloat", "akceprovatelna jsou pouze kladna cisla", arg, name);
    errno = ERANGE;
    return (float)0;
  }

  if((flags & ERN_GTZERO) && res <= 0){
    gnFail("getFloat", "akceprovatelna jsou pouze nezaporna cisla", arg, name);
    errno = ERANGE;
    return (float)0;
  }

  return res;

}

double getDouble(const char *arg, int flags, const char *name)
{
  double res;
  char *endptr;

  errno = 0;
  res = strtod(arg, &endptr);

  if(errno != 0){
    gnFail("getFloat", "strtof() failed", (char *)NULL, name);
    return (double)0;
  }

  if(*endptr != '\0'){
    gnFail("getFloat", "nenumericky znak", arg, name);
    errno = EINVAL;
    return (double)0;
  }

  if((flags & ERN_NONNEG) && res < 0){
    gnFail("getFloat", "akceprovatelna jsou pouze kladna cisla", arg, name);
    errno = ERANGE;
    return (double)0;
  }

  if((flags & ERN_GTZERO) && res <= 0){
    gnFail("getFloat", "akceprovatelna jsou pouze nezaporna cisla", arg, name);
    errno = ERANGE;
    return (double)0;
  }

  return res;
}

long double getLDouble(const char *arg, int flags, const char *name)
{
  long double res;
  char *endptr;

  errno = 0;
  res = strtold(arg, &endptr);

  if(errno != 0){
    gnFail("getFloat", "strtof() failed", (char *)NULL, name);
    return (long double)0;
  }

  if(*endptr != '\0'){
    gnFail("getFloat", "nenumericky znak", arg, name);
    errno = EINVAL;
    return (long double)0;
  }

  if((flags & ERN_NONNEG) && res < 0){
    gnFail("getFloat", "akceprovatelna jsou pouze kladna cisla", arg, name);
    errno = ERANGE;
    return (long double)0;
  }

  if((flags & ERN_GTZERO) && res <= 0){
    gnFail("getFloat", "akceprovatelna jsou pouze nezaporna cisla", arg, name);
    errno = ERANGE;
    return (long double)0;
  }
}


int getStr_FromInt(char **str, const int digit, const int num, int flags, const char *name)
{

    if((flags & ERN_NONNEG) && num < 0){
      gnFail("getFloat", "akceprovatelna jsou pouze kladna cisla", "argument: num", name);
      errno = ERANGE;
      return(-1);
    }

    if((flags & ERN_GTZERO) && num <= 0){
      gnFail("getFloat", "akceprovatelna jsou pouze nezaporna cisla", "argument: num", name);
      errno = ERANGE;
      return(-1);
    }

    if(NULL == ((*str) = (char *)malloc(sizeof(char)*(digit+1)))){/*+1 because of \0 character*/
      errExit(ER_WEXIT, "getStr_FromInt malloc()");
    }

    if(0 > snprintf((*str), digit + 1, ((flags & ERN_HEX)? "%x": ((flags & ERN_OCT)? "%o": "%d")), num)){/*+1 because of \0 character*/
       errExit(ER_WEXIT, "cannot convert to the string");
    }
    return(0);
}

int getStr_FromLong(char **str, const int digit, const long num, int flags, const char *name)
{

  if((flags & ERN_NONNEG) && num < 0){
    gnFail("getFloat", "akceprovatelna jsou pouze kladna cisla", "argument num", name);
    errno = ERANGE;
    return(-1);
  }

  if((flags & ERN_GTZERO) && num <= 0){
    gnFail("getFloat", "akceprovatelna jsou pouze nezaporna cisla", "argument num", name);
    errno = ERANGE;
    return(-1);
  }

  if(NULL == ((*str) = (char *)malloc((digit + 1) * sizeof(char)))){/*+1 because of \0 character*/
    errExit(ER_WEXIT, "getStr_FromInt malloc()");
  }

  if(0 > snprintf((*str), digit + 1,"%ld", num)){/*+1 because of \0 character*/
     errExit(ER_WEXIT, "cannot convert to the string");
  }

  return(0);
}


int getStr_FromFloat(char **str, const int digit, const float num, int flags, const char *name)
{

  if((flags & ERN_NONNEG) && num < 0){
    gnFail("getFloat", "akceprovatelna jsou pouze kladna cisla", "argument num", name);
    errno = ERANGE;
    return(-1);
  }

  if((flags & ERN_GTZERO) && num <= 0){
    gnFail("getFloat", "akceprovatelna jsou pouze nezaporna cisla", "argument num", name);
    errno = ERANGE;
    return(-1);
  }

  if(NULL == ((*str) = (char *)malloc(sizeof(char)*(digit+1)))){/*+1 because of \0 character*/
    errExit(ER_WEXIT, "getStr_FromInt malloc()");
  }

  if(0 > snprintf((*str), digit + 1, ((flags & ERN_EXP)?"%e": "%f"),num)){/*+1 because of \0 character*/
     errExit(ER_WEXIT, "cannot convert to the string");
  }

  return(0);
}

int getStr_FromDouble(char **str, const int digit, const double num, int flags, const char *name)
{

  if((flags & ERN_NONNEG) && num < 0){
    gnFail("getFloat", "akceprovatelna jsou pouze kladna cisla", "argument num", name);
    errno = ERANGE;
    return(-1);
  }

  if((flags & ERN_GTZERO) && num <= 0){
    gnFail("getFloat", "akceprovatelna jsou pouze nezaporna cisla", "argument num", name);
    errno = ERANGE;
    return(-1);
  }

  REALLOC((*str), (digit + 1), sizeof(char), "getStr_FromDouble", FALSE);

  if(0 > snprintf((*str), digit + 1,((flags & ERN_EXP)?"%e": "%f"), num)){/*+1 because of \0 character*/
     gnFail("getStr_FromInt","canot convert to the string", (char *)NULL, name);
     errExit(ER_WEXIT, "cannot convert to the string");
  }
  return(0);
}
