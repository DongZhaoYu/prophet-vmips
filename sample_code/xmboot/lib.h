#include <stddef.h>

/* Macros */
#ifndef NULL
#define NULL ((void *) 0L)
#endif

/* Terminal modes */
#define ECHO   0x0001   /* Echo incoming characters */
#define ONLCR  0x0002   /* Translate \n to \r\n on output */
#define ICRNL  0x0004   /* Translate \r on input to \n */
#define COOKED 0x0007   /* All the good terminal modes */

/* Function prototypes */
void set_echo (int onoff);
void term_enable (int mode);
void term_disable (int mode);
int getchar (void);
int putchar (int ch);
void puts_nonl (const char *buf);
char *gets (char *buf);
int puts (const char *buf);
int strcmp (const char *s1, const char *s2);
int strncmp (const char *s1, const char *s2, size_t n);
size_t strlen (const char *s);
char toupper (char c);
char tolower (char c);
int isspace (const char c);
int isdigit (const char c);
int isprint (const char c);
long strtol (const char *s, char **endptr, int radix);
int printf (const char *fmt, ...);
char *strcpy (const char *s1, const char *s2);
char *strncpy (char *s1, const char *s2, size_t n);
void *memcpy (void *dest, const void *src, size_t n);
void *memmove (void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
char *strcat(char *dest, const char *src);
int atoi(const char *nptr);
