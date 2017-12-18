#define __STDC_VERSION__ 199901L
void funlockfile(FILE *);
void flockfile(FILE *);
int vasprintf(char**buffer, const char *format, va_list argptr);
int asprintf(char**buffer, const char *format, va_list argptr);
#define __func__ __FUNCTION__

