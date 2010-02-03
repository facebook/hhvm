<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('override_function', Boolean,
  array('name' => String,
        'args' => String,
        'code' => String));

f('rename_function', Boolean,
  array('orig_name' => String,
        'new_name' => String));

f('apd_set_browser_trace', NULL);

f('apd_set_pprof_trace', String,
  array('dumpdir' => array(String, 'null_string'),
        'frament' => array(String, 'null_string')));

f('apd_set_session_trace_socket', Boolean,
  array('ip_or_filename' => String,
        'domain' => Int32,
        'port' => Int32,
        'mask' => Int32));

f('apd_stop_trace', NULL);

f('apd_breakpoint', Boolean);

f('apd_continue', Boolean);

f('apd_echo', Boolean,
  array('output' => String));
