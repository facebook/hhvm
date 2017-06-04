#ifndef _DBF_HEAD_T
#define _DBF_HEAD_T

dbhead_t *get_dbf_head(int fd);
void free_dbf_head(dbhead_t *dbh);
int put_dbf_head(dbhead_t *dbh);
int get_dbf_field(dbhead_t *dbh, dbfield_t *dbf);
int put_dbf_field(dbhead_t *dbh, dbfield_t *dbf);
void put_dbf_info(dbhead_t *dbh);
char *get_dbf_f_fmt(dbfield_t *dbf);
dbhead_t *dbf_open(const char *dp, int o_flags);
void dbf_head_info(dbhead_t *dbh);

#endif
