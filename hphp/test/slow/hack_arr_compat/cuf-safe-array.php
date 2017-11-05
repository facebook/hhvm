<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

$global_var = 123;
function &ret_by_ref() {
  global $global_var;
  return $global_var;
}

var_dump(fb_call_user_func_safe('ret_by_ref'));
