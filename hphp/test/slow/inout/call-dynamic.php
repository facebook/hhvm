<?hh

function foo(inout $x) {
  $x = 42;
}

function bar(inout $a, inout $b) {
  list($a, $b) = array($b, $a);
  return $a + $b;
}

function baz(inout $q) {
  $q = debug_backtrace()[0]['function'];
  return 12;
}

function main($a, $b, $c) {
  $foo = 0;
  $bar1 = 'a';
  $bar2 = 'b';
  $baz = null;
  $a(inout $foo);
  $b(inout $bar1, inout $bar2);
  $c(inout $baz);

  var_dump($foo, $bar1, $bar2, $baz);
}

if (!isset($x)) $x = 'foo';
if (!isset($y)) $y = 'bar';
if (!isset($z)) $z = 'baz';
main($x, $y, $z);
