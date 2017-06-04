#ifndef _DBF_MISC_H
#define _DBF_MISC_H

void put_long(char *cp, long lval);
long get_long(char *cp);
int get_short(char *cp);
void put_short(char *cp, int sval);
void put_double(char *cp, double fval);
double get_double(char *cp);
void copy_fill(char *dp, char *sp, int len);
void copy_crimp(char *dp, char *sp, int len);
void db_set_date(char *cp, int year, int month, int day);
int db_date_year(char *cp);
int db_date_month(char *cp);
int db_date_day(char *cp);
char *db_cur_date(char *cp);

#endif
