<?php

function main() {
  $a = array(1);
  $b =& $a[0];
  var_dump(call_user_func_array('foo', $a));
  var_dump(fb_call_user_func_safe('foo', $a));
  var_dump(fb_call_user_func_array_safe('foo', $a));
  var_dump(fb_call_user_func_safe_return('foo', 'foobar', $a));
  var_dump($a);

  $a = array(1);
  var_dump(call_user_func_array('bar', $a));
  var_dump($a);
  var_dump(array(1));

  $a = array(1);
  var_dump(call_user_func_array('baz', $a));
  var_dump(call_user_func('baz', $a));
  var_dump(fb_call_user_func_safe('baz', $a));
  var_dump(fb_call_user_func_array_safe('baz', $a));
  var_dump(fb_call_user_func_safe_return('baz', 'foobar', $a));

}

function foo($x) {
  var_dump($x);
  $x = 2;
  return 1;
}
function bar(&$x) {
  $x = 2;
  return 1;
}

main();
