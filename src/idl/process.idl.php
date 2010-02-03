<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////
// process control functions

f('pcntl_alarm', Int32,
  array('seconds' => Int32));

f('pcntl_exec', NULL,
  array('path' => String,
        'args' => array(StringVec, 'null_array'),
        'envs' => array(StringVec, 'null_array')));

f('pcntl_fork', Int32);

f('pcntl_getpriority', Variant,
  array('pid' => array(Int32, '0'),
        'process_identifier' => array(Int32, '0')));

f('pcntl_setpriority', Boolean,
  array('priority' => Int32,
        'pid' => array(Int32, '0'),
        'process_identifier' => array(Int32, '0')));

f('pcntl_signal', Boolean,
  array('signo' => Int32,
        'handler' => Variant,
        'restart_syscalls' => array(Boolean, 'true')));

f('pcntl_wait', Int32,
  array('status' => Int32 | Reference,
        'options' => array(Int32, '0')));

f('pcntl_waitpid', Int32,
  array('pid' => Int32,
        'status' => Int32 | Reference,
        'options' => array(Int32, '0')));

f('pcntl_wexitstatus', Int32,   array('status' => Int32));
f('pcntl_wifexited',   Boolean, array('status' => Int32));
f('pcntl_wifsignaled', Boolean, array('status' => Int32));
f('pcntl_wifstopped',  Boolean, array('status' => Int32));
f('pcntl_wstopsig',    Int32,   array('status' => Int32));
f('pcntl_wtermsig',    Int32,   array('status' => Int32));

// added in PHP 5.3.0
f('pcntl_signal_dispatch', Boolean, array());

///////////////////////////////////////////////////////////////////////////////
// program execution functions

f('shell_exec', String,
  array('cmd' => String));

f('exec', String,
  array('command' => String,
        'output' => array(VariantMap | Reference, 'null'),
        'return_var' => array(Int32 | Reference, 'null')));

f('passthru', NULL,
  array('command' => String,
        'return_var' => array(Int32 | Reference, 'null')));

f('system', String,
  array('command' => String,
        'return_var' => array(Int32 | Reference, 'null')));

f('proc_open', Variant,
  array('cmd' => String,
        'descriptorspec' => VariantVec,
        'pipes' => VariantVec | Reference,
        'cwd' => array(String, 'null_string'),
        'env' => array(Variant, 'null_variant'),
        'other_options' => array(Variant, 'null_variant')));

f('proc_terminate', Boolean,
  array('process' => Resource,
        'signal' => array(Int32, '0')));

f('proc_close', Int32,
  array('process' => Resource));

f('proc_get_status', VariantMap,
  array('process' => Resource));

f('proc_nice', Boolean,
  array('increment' => Int32));

f('escapeshellarg', String,
  array('arg' => String));

f('escapeshellcmd', String,
  array('command' => String));
