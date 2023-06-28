<?hh


// For simple cases, see php5_pcntl_003.php

<<__EntryPoint>>
function main_sigprocmask_edge_cases() :mixed{
$oldset = null;
print("Correct usage, two args\n");
var_dump(pcntl_sigprocmask(SIG_BLOCK, varray[SIGHUP], inout $oldset));

print("Invalid \$how\n");
var_dump(pcntl_sigprocmask(PHP_INT_MAX, varray[], inout $oldset));

print("Invalid signal\n");
var_dump(pcntl_sigprocmask(SIG_SETMASK, varray[1337], inout $oldset));

print("Invalid byref arg\n");
$oldset = new stdClass();
var_dump(pcntl_sigprocmask(SIG_SETMASK, varray[SIGHUP], inout $oldset));
var_dump(is_array($oldset));
}
