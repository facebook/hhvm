#if EXT_TYPE == 0
"socket_create", T(Variant), S(0), "domain", T(Int32), NULL, S(0), "type", T(Int32), NULL, S(0), "protocol", T(Int32), NULL, S(0), NULL, S(0), 
"socket_create_listen", T(Variant), S(0), "port", T(Int32), NULL, S(0), "backlog", T(Int32), "128", S(0), NULL, S(0), 
"socket_create_pair", T(Boolean), S(0), "domain", T(Int32), NULL, S(0), "type", T(Int32), NULL, S(0), "protocol", T(Int32), NULL, S(0), "fd", T(Variant), NULL, S(1), NULL, S(0), 
"socket_get_option", T(Variant), S(0), "socket", T(Object), NULL, S(0), "level", T(Int32), NULL, S(0), "optname", T(Int32), NULL, S(0), NULL, S(0), 
"socket_getpeername", T(Boolean), S(0), "socket", T(Object), NULL, S(0), "address", T(Variant), NULL, S(1), "port", T(Variant), "null", S(1), NULL, S(0), 
"socket_getsockname", T(Boolean), S(0), "socket", T(Object), NULL, S(0), "address", T(Variant), NULL, S(1), "port", T(Variant), "null", S(1), NULL, S(0), 
"socket_set_block", T(Boolean), S(0), "socket", T(Object), NULL, S(0), NULL, S(0), 
"socket_set_nonblock", T(Boolean), S(0), "socket", T(Object), NULL, S(0), NULL, S(0), 
"socket_set_option", T(Boolean), S(0), "socket", T(Object), NULL, S(0), "level", T(Int32), NULL, S(0), "optname", T(Int32), NULL, S(0), "optval", T(Variant), NULL, S(0), NULL, S(0), 
"socket_connect", T(Boolean), S(0), "socket", T(Object), NULL, S(0), "address", T(String), NULL, S(0), "port", T(Int32), "0", S(0), NULL, S(0), 
"socket_bind", T(Boolean), S(0), "socket", T(Object), NULL, S(0), "address", T(String), NULL, S(0), "port", T(Int32), "0", S(0), NULL, S(0), 
"socket_listen", T(Boolean), S(0), "socket", T(Object), NULL, S(0), "backlog", T(Int32), "0", S(0), NULL, S(0), 
"socket_select", T(Variant), S(0), "read", T(Variant), NULL, S(1), "write", T(Variant), NULL, S(1), "except", T(Variant), NULL, S(1), "vtv_sec", T(Variant), NULL, S(0), "tv_usec", T(Int32), "0", S(0), NULL, S(0), 
"socket_server", T(Variant), S(0), "hostname", T(String), NULL, S(0), "port", T(Int32), "-1", S(0), "errnum", T(Variant), "null", S(1), "errstr", T(Variant), "null", S(1), NULL, S(0), 
"socket_accept", T(Variant), S(0), "socket", T(Object), NULL, S(0), NULL, S(0), 
"socket_read", T(Variant), S(0), "socket", T(Object), NULL, S(0), "length", T(Int32), NULL, S(0), "type", T(Int32), "0", S(0), NULL, S(0), 
"socket_write", T(Variant), S(0), "socket", T(Object), NULL, S(0), "buffer", T(String), NULL, S(0), "length", T(Int32), "0", S(0), NULL, S(0), 
"socket_send", T(Variant), S(0), "socket", T(Object), NULL, S(0), "buf", T(String), NULL, S(0), "len", T(Int32), NULL, S(0), "flags", T(Int32), NULL, S(0), NULL, S(0), 
"socket_sendto", T(Variant), S(0), "socket", T(Object), NULL, S(0), "buf", T(String), NULL, S(0), "len", T(Int32), NULL, S(0), "flags", T(Int32), NULL, S(0), "addr", T(String), NULL, S(0), "port", T(Int32), "0", S(0), NULL, S(0), 
"socket_recv", T(Variant), S(0), "socket", T(Object), NULL, S(0), "buf", T(Variant), NULL, S(1), "len", T(Int32), NULL, S(0), "flags", T(Int32), NULL, S(0), NULL, S(0), 
"socket_recvfrom", T(Variant), S(0), "socket", T(Object), NULL, S(0), "buf", T(Variant), NULL, S(1), "len", T(Int32), NULL, S(0), "flags", T(Int32), NULL, S(0), "name", T(Variant), NULL, S(1), "port", T(Variant), "0", S(1), NULL, S(0), 
"socket_shutdown", T(Boolean), S(0), "socket", T(Object), NULL, S(0), "how", T(Int32), "0", S(0), NULL, S(0), 
"socket_close", T(Void), S(0), "socket", T(Object), NULL, S(0), NULL, S(0), 
"socket_strerror", T(String), S(0), "errnum", T(Int32), NULL, S(0), NULL, S(0), 
"socket_last_error", T(Int32), S(0), "socket", T(Object), "null_object", S(0), NULL, S(0), 
"socket_clear_error", T(Void), S(0), "socket", T(Object), "null_object", S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
