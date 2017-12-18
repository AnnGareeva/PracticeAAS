#include <stdio.h>
#include <stdlib.h>
void funlockfile(FILE * f) {}
void flockfile(FILE * f) {}
#define MAX_BUFFER_SIZE 10*1024
int vasprintf(char**buffer, const char *format, va_list argptr) {
	*buffer=(char*)malloc(MAX_BUFFER_SIZE);
	return vsprintf(*buffer,format,argptr);
};
int asprintf(char**buffer, const char *format, va_list argptr) {
	*buffer=(char*)malloc(MAX_BUFFER_SIZE);
	return sprintf(*buffer,format,argptr);
};
