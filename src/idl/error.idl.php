<?php

include_once 'base.php';

///////////////////////////////////////////////////////////////////////////////

f('debug_backtrace', StringVec, array());

f('debug_print_backtrace', null, array());

f('error_get_last', StringVec);

f('error_log', Boolean,
  array('message' => String,
        'message_type' => array(Int32, '0'),
        'destination' => array(String, 'null_string'),
        'extra_headers' => array(String, 'null_string')));

f('error_reporting', Int32,
  array('level' => array(Variant, 'null')));

f('restore_error_handler', Boolean);

f('restore_exception_handler', Boolean);

f('set_error_handler', Variant,
  array('error_handler' => Variant,
        'error_types' => array(Int32, '0')));

f('set_exception_handler', String,
  array('exception_handler' => String));

f('hphp_set_error_page', null,
  array('page' => String));

f('trigger_error', Boolean,
  array('error_msg' => String,
        'error_type' => array(Int32, 'k_E_USER_NOTICE')));

f('user_error', Boolean,
  array('error_msg' => String,
        'error_type' => array(Int32, 'k_E_USER_NOTICE')));
