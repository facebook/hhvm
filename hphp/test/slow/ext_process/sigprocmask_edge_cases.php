<?php

// For simple cases, see php5_pcntl_003.php

print("Correct usage, two args\n");
var_dump(pcntl_sigprocmask(SIG_BLOCK, [SIGHUP]));

print("Invalid \$how\n");
var_dump(pcntl_sigprocmask(PHP_INT_MAX, []));

print("Invalid signal\n");
var_dump(pcntl_sigprocmask(SIG_SETMASK, [1337]));

print("Invalid byref arg\n");
$oldset = new stdClass();
var_dump(pcntl_sigprocmask(SIG_SETMASK, [SIGHUP], $oldset));
var_dump(is_array($oldset));
