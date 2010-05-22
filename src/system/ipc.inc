#if EXT_TYPE == 0
"ftok", T(Int64), S(0), "pathname", T(String), NULL, S(0), "proj", T(String), NULL, S(0), NULL, S(0), 
"msg_get_queue", T(Variant), S(0), "key", T(Int64), NULL, S(0), "perms", T(Int64), "0666", S(0), NULL, S(0), 
"msg_send", T(Boolean), S(0), "queue", T(Object), NULL, S(0), "msgtype", T(Int64), NULL, S(0), "message", T(Variant), NULL, S(0), "serialize", T(Boolean), "true", S(0), "blocking", T(Boolean), "true", S(0), "errorcode", T(Variant), "null", S(1), NULL, S(0), 
"msg_receive", T(Boolean), S(0), "queue", T(Object), NULL, S(0), "desiredmsgtype", T(Int64), NULL, S(0), "msgtype", T(Variant), NULL, S(1), "maxsize", T(Int64), NULL, S(0), "message", T(Variant), NULL, S(1), "unserialize", T(Boolean), "true", S(0), "flags", T(Int64), "0", S(0), "errorcode", T(Variant), "null", S(1), NULL, S(0), 
"msg_remove_queue", T(Boolean), S(0), "queue", T(Object), NULL, S(0), NULL, S(0), 
"msg_set_queue", T(Boolean), S(0), "queue", T(Object), NULL, S(0), "data", T(Array), NULL, S(0), NULL, S(0), 
"msg_stat_queue", T(Array), S(0), "queue", T(Object), NULL, S(0), NULL, S(0), 
"sem_acquire", T(Boolean), S(0), "sem_identifier", T(Object), NULL, S(0), NULL, S(0), 
"sem_get", T(Variant), S(0), "key", T(Int64), NULL, S(0), "max_acquire", T(Int64), "1", S(0), "perm", T(Int64), "0666", S(0), "auto_release", T(Boolean), "true", S(0), NULL, S(0), 
"sem_release", T(Boolean), S(0), "sem_identifier", T(Object), NULL, S(0), NULL, S(0), 
"sem_remove", T(Boolean), S(0), "sem_identifier", T(Object), NULL, S(0), NULL, S(0), 
"shm_attach", T(Variant), S(0), "shm_key", T(Int64), NULL, S(0), "shm_size", T(Int64), "10000", S(0), "shm_flag", T(Int64), "0666", S(0), NULL, S(0), 
"shm_detach", T(Boolean), S(0), "shm_identifier", T(Int64), NULL, S(0), NULL, S(0), 
"shm_remove", T(Boolean), S(0), "shm_identifier", T(Int64), NULL, S(0), NULL, S(0), 
"shm_get_var", T(Variant), S(0), "shm_identifier", T(Int64), NULL, S(0), "variable_key", T(Int64), NULL, S(0), NULL, S(0), 
"shm_put_var", T(Boolean), S(0), "shm_identifier", T(Int64), NULL, S(0), "variable_key", T(Int64), NULL, S(0), "variable", T(Some), NULL, S(0), NULL, S(0), 
"shm_remove_var", T(Boolean), S(0), "shm_identifier", T(Int64), NULL, S(0), "variable_key", T(Int64), NULL, S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
