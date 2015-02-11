<?hh

# This test produces different results on different runtimes (and different
# minor versions of those runtimes), so if you've changed the output, just make
# sure it's on purpose.

$x = 1;

function &byref() {
  return $GLOBALS['x'];
}

function main() {
  global $x;

  $y = &byref();
  $y = 2;
  var_dump($x);

  $z =& call_user_func('byref');
  $z = 3;
  var_dump($x);

  $cuf = 'call_user_func';
  $w =& $cuf('byref');
  $w = 4;
  var_dump($x);

  $rf = new ReflectionFunction("byref");
  $w =& $rf->invokeArgs(array());
  $w = 5;
  var_dump($x);
}
main();
