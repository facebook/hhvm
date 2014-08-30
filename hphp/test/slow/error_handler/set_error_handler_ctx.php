<?php

$globalStr = 'Global A';
$globalArr = array(1, 2, 3);

set_error_handler(function($errno, $errstr, $errfile, $errline, $errcontext) {
  $keys = array_keys($errcontext);
  $check_keys = array();
  if ($errstr == 'ErrorLocal') {
    $check_keys = array('param1', 'param2', 'local', 'globalStr');
  } else if ($errstr == 'ErrorGlobal') {
    $check_keys = array('GLOBALS', 'globalStr', 'globalArr', '_SERVER');
  } else if ($errstr == 'ErrorClosure') {
    $check_keys = array('local', 'cl_param');
  }

  var_dump(array_intersect($check_keys, $keys));
});


function func($param1, $param2='default')
{
  $local = 'localvar';
  global $globalStr;
  trigger_error('ErrorLocal', E_USER_WARNING);

  $closure = function($cl_param) use($local) {
    trigger_error('ErrorClosure', E_USER_WARNING);
  };

  $closure('closure_param');
}

trigger_error('ErrorGlobal', E_USER_WARNING);
func('param1');
