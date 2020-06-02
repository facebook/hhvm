<?hh
function foo() {
  $GLOBALS['foo'] = 1;
  unset($GLOBALS['foo']);
  var_dump(array_key_exists('foo', $GLOBALS['GLOBALS']));
  $g = darray['foo' => 1];
  unset($g['foo']);
  var_dump(array_key_exists('foo', $g));
  var_dump(array_key_exists('a', $GLOBALS['GLOBALS']));
  unset($GLOBALS['a']);
  var_dump(array_key_exists('a', $GLOBALS['GLOBALS']));
}
<<__EntryPoint>>
function entrypoint_1114(): void {

  $GLOBALS['a'] = 1;
  foo();
}
