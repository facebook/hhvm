#ifdef DEBUG
#define debug(s) Printf.eprintf "[%S %i] %s\n%!" __FILE__ __LINE__ s
#else
#define debug(s) ()
#endif

debug("test")
