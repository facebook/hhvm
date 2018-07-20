<?hh

function foo(inout $x, $y, ...$z) {
  $x[] = $y;
  foreach ($z as $v) $x[] = $v;
}

function bar(inout $x, ...$z) {
  $r = $x[count($x) - 1];
  unset($x[count($x) - 1]);
  if (count($z) > 0) var_dump("wat");
  return $r;
}

function main() {
  $x = [1, 2, 3];
  foo(inout $x, 24);
  foo(inout $x, 42, 'apple', 200);
  foo(&$x, 8, 99, 'green');
  foo(&$x, 16);
  var_dump($x);
  var_dump(bar(&$x));
  var_dump(bar(inout $x));
  var_dump(bar(&$x));
  var_dump(bar(inout $x));
  var_dump($x);
}

main();
