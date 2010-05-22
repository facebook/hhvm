#if EXT_TYPE == 0
"pcntl_alarm", T(Int32), S(0), "seconds", T(Int32), NULL, S(0), NULL, S(0), 
"pcntl_exec", T(Void), S(0), "path", T(String), NULL, S(0), "args", T(Array), "null_array", S(0), "envs", T(Array), "null_array", S(0), NULL, S(0), 
"pcntl_fork", T(Int32), S(0), NULL, S(0), 
"pcntl_getpriority", T(Variant), S(0), "pid", T(Int32), "0", S(0), "process_identifier", T(Int32), "0", S(0), NULL, S(0), 
"pcntl_setpriority", T(Boolean), S(0), "priority", T(Int32), NULL, S(0), "pid", T(Int32), "0", S(0), "process_identifier", T(Int32), "0", S(0), NULL, S(0), 
"pcntl_signal", T(Boolean), S(0), "signo", T(Int32), NULL, S(0), "handler", T(Variant), NULL, S(0), "restart_syscalls", T(Boolean), "true", S(0), NULL, S(0), 
"pcntl_wait", T(Int32), S(0), "status", T(Variant), NULL, S(1), "options", T(Int32), "0", S(0), NULL, S(0), 
"pcntl_waitpid", T(Int32), S(0), "pid", T(Int32), NULL, S(0), "status", T(Variant), NULL, S(1), "options", T(Int32), "0", S(0), NULL, S(0), 
"pcntl_wexitstatus", T(Int32), S(0), "status", T(Int32), NULL, S(0), NULL, S(0), 
"pcntl_wifexited", T(Boolean), S(0), "status", T(Int32), NULL, S(0), NULL, S(0), 
"pcntl_wifsignaled", T(Boolean), S(0), "status", T(Int32), NULL, S(0), NULL, S(0), 
"pcntl_wifstopped", T(Boolean), S(0), "status", T(Int32), NULL, S(0), NULL, S(0), 
"pcntl_wstopsig", T(Int32), S(0), "status", T(Int32), NULL, S(0), NULL, S(0), 
"pcntl_wtermsig", T(Int32), S(0), "status", T(Int32), NULL, S(0), NULL, S(0), 
"pcntl_signal_dispatch", T(Boolean), S(0), NULL, S(0), 
"shell_exec", T(String), S(0), "cmd", T(String), NULL, S(0), NULL, S(0), 
"exec", T(String), S(0), "command", T(String), NULL, S(0), "output", T(Variant), "null", S(1), "return_var", T(Variant), "null", S(1), NULL, S(0), 
"passthru", T(Void), S(0), "command", T(String), NULL, S(0), "return_var", T(Variant), "null", S(1), NULL, S(0), 
"system", T(String), S(0), "command", T(String), NULL, S(0), "return_var", T(Variant), "null", S(1), NULL, S(0), 
"proc_open", T(Variant), S(0), "cmd", T(String), NULL, S(0), "descriptorspec", T(Array), NULL, S(0), "pipes", T(Variant), NULL, S(1), "cwd", T(String), "null_string", S(0), "env", T(Variant), "null_variant", S(0), "other_options", T(Variant), "null_variant", S(0), NULL, S(0), 
"proc_terminate", T(Boolean), S(0), "process", T(Object), NULL, S(0), "signal", T(Int32), "0", S(0), NULL, S(0), 
"proc_close", T(Int32), S(0), "process", T(Object), NULL, S(0), NULL, S(0), 
"proc_get_status", T(Array), S(0), "process", T(Object), NULL, S(0), NULL, S(0), 
"proc_nice", T(Boolean), S(0), "increment", T(Int32), NULL, S(0), NULL, S(0), 
"escapeshellarg", T(String), S(0), "arg", T(String), NULL, S(0), NULL, S(0), 
"escapeshellcmd", T(String), S(0), "command", T(String), NULL, S(0), NULL, S(0), 
#elif EXT_TYPE == 1
#elif EXT_TYPE == 2
#elif EXT_TYPE == 3

#endif
