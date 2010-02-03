<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('posix_access',     Boolean,
  array('file' => String,
        'mode' => array(Int32, '0')));

f('posix_ctermid',    String);
f('posix_get_last_error', Int32);
f('posix_getcwd',     String);
f('posix_getegid',    Int32);
f('posix_geteuid',    Int32);
f('posix_getgid',     Int32);
f('posix_getgrgid',   Variant,    array('gid' => Int32));
f('posix_getgrnam',   Variant,    array('name' => String));
f('posix_getgroups',  Variant);
f('posix_getlogin',   Variant);
f('posix_getpgid',    Variant,    array('pid' => Int32));
f('posix_getpgrp',    Int32);
f('posix_getpid',     Int32);
f('posix_getppid',    Int32);
f('posix_getpwnam',   Variant,    array('username' => String));
f('posix_getpwuid',   Variant,    array('uid' => Int32));
f('posix_getrlimit',  Variant);
f('posix_getsid',     Variant,    array('pid' => Int32));
f('posix_getuid',     Int32);
f('posix_initgroups', Boolean,
  array('name' => String,
        'base_group_id' => Int32));

f('posix_isatty',     Boolean,    array('fd' => Variant));
f('posix_kill',       Boolean,    array('pid' => Int32, 'sig' => Int32));
f('posix_mkfifo',     Boolean,
  array('pathname' => String,
        'mode' => Int32));

f('posix_mknod',      Boolean,
  array('pathname' => String,
        'mode' => Int32,
        'major' => array(Int32, '0'),
        'minor' => array(Int32, '0')));

f('posix_setegid',    Boolean,   array('gid' => Int32));
f('posix_seteuid',    Boolean,   array('uid' => Int32));
f('posix_setgid',     Boolean,   array('gid' => Int32));
f('posix_setpgid',    Boolean,   array('pid' => Int32, 'pgid' => Int32));
f('posix_setsid',     Int32);
f('posix_setuid',     Boolean,   array('uid' => Int32));
f('posix_strerror',   String,    array('errnum' => Int32));
f('posix_times',      Variant);
f('posix_ttyname',    Variant,   array('fd' => Variant));
f('posix_uname',      Variant);
