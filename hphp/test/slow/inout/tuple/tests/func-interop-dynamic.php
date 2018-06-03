<?hh

function foo(inout $x) {
  $x = 42;
  $bt = array_slice(debug_backtrace(), 0, 2);
  echo implode(', ', array_map($a ==> $a['function'], $bt))."\n";
  return 'Hello';
}

function bar(&$y) {
  $y = 9;
  $bt = array_slice(debug_backtrace(), 0, 2);
  echo implode(', ', array_map($a ==> $a['function'], $bt))."\n";
  return 'world!';
}

function main($foo, $bar) {
  $a = null;
  $b = null;
  $c = null;
  $g1 = $foo(inout $a);
  $g2 = $foo(&$b);
  $g3 = $foo(&$c);
  var_dump($a, $b, $c);

  $x = null;
  $y = null;
  $z = null;
  $h1 = $bar(&$x);
  $h2 = $bar(&$y);
  $h3 = $bar(inout $z);
  var_dump($x, $y, $z);

  echo "$g1, $h1\n";
  echo "$g2, $h2\n";
  echo "$g2, $h2\n";
}

main('foo', 'bar');
