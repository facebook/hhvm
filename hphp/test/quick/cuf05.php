<?hh

function main() {
  $a = array(1);
  $b =& $a[0];
  var_dump(call_user_func_array('foo', $a));
  var_dump(call_user_func('foo', $a));
  var_dump($a);

  $a = array(1);
  var_dump(call_user_func_array('baz', $a));
  var_dump(call_user_func('baz', $a));
}

function foo($x) {
  var_dump($x);
  $x = 2;
  return 1;
}

main();
