#ifndef _DBF_REC_H
#define _DBF_REC_H

char *get_dbf_record(dbhead_t *dbh, long rec_num);
long put_dbf_record(dbhead_t *dbh, long rec_num, char *cp);
int put_piece(dbhead_t *dbh, long offset, char *cp, int len);
int del_dbf_record(dbhead_t *dbh, long rec_num);
void pack_dbf(dbhead_t *dbh);
char *get_field_val(char *rp, dbfield_t *fldp, char *cp);
void put_field_val(char *rp, dbfield_t *fldp, char *cp);
void out_rec(dbhead_t *dbh, dbfield_t *dbf, char *cp);
int is_valid_rec(char *cp);
char *dbf_get_next(dbhead_t *dbh);

#endif
