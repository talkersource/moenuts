/*------------------------- Prototypes for figlet.c -------------------------*/
#ifndef _FIGLET_SOURCE
  #define EXTERN extern
#else
  #define EXTERN
#endif
EXTERN void write_textfig(char *,char *,char *);
EXTERN void write_bcfig(char *);
#ifdef __STDC__
  EXTERN char *myalloc(size_t size);
#else
  EXTERN char *myalloc(int);
#endif
EXTERN void skiptoeol(FILE *);
EXTERN void fclearline(void);
EXTERN void readfontchar(FILE *,inchr,char *,int);
EXTERN int readfont(char *);
EXTERN void getletter(inchr);
EXTERN int addchar(inchr);
EXTERN void putstring(char *);
EXTERN void printline(void);
EXTERN void splitline(void);
EXTERN void figlet(UR_OBJECT,char *);
#undef EXTERN
/*--------------------- end of figlet prototypes ----------------------------*/
