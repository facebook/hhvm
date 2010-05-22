<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('connection_aborted', Int32);
f('connection_status', Int32);
f('connection_timeout', Int32);

f('constant', Variant,
  array('name' => String));

f('define', Boolean,
  array('name' => String,
        'value' => Variant,
        'case_insensitive' => array(Boolean, 'false')));

f('defined', Boolean,
  array('name' => String));

f('die', Variant,
  array('status' => array(Variant, 'null_variant')));

f('exit', Variant,
  array('status' => array(Variant, 'null_variant')));

f('eval', Variant,
  array('code_str' => String));

f('get_browser', Variant,
  array('user_agent' => array(String, 'null_string'),
        'return_array' => array(Boolean, 'false')));

f('__halt_compiler');

f('highlight_file', Variant,
  array('filename' => String,
        'ret' => array(Boolean, 'false')));

f('show_source', Variant,
  array('filename' => String,
        'ret' => array(Boolean, 'false')));

f('highlight_string', Variant,
  array('str' => String,
        'ret' => array(Boolean, 'false')));

f('ignore_user_abort', Int32,
  array('setting' => array(Boolean, 'false')));

f('pack', Variant,
  array('format' => String),
  VariableArguments);

f('php_check_syntax', Boolean,
  array('filename' => String,
        'error_message' => array(String | Reference, 'null')));

f('php_strip_whitespace', String,
  array('filename' => String));

f('sleep', Int32,
  array('seconds' => Int32));

f('usleep', NULL,
  array('micro_seconds' => Int32));

f('time_nanosleep', Variant,
  array('seconds' => Int32,
        'nanoseconds' => Int32));

f('time_sleep_until', Boolean,
  array('timestamp' => Double));

f('uniqid', String,
  array('prefix' => array(String, 'null_string'),
        'more_entropy' => array(Boolean, 'false')));

f('unpack', Variant,
  array('format' => String,
        'data' => String));

f('sys_getloadavg', VariantMap);

f('token_get_all', VariantMap,
  array('source' => String));

f('token_name', String,
  array('token' => Int64));


