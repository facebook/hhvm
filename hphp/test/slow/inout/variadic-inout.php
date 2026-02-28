<?hh

function foo(inout $x, $y, ...$z) :mixed{
  $x[] = $y;
  foreach ($z as $v) $x[] = $v;
}

function bar(inout $x, ...$z) :mixed{
  $r = $x[count($x) - 1];
  unset($x[count($x) - 1]);
  if (count($z) > 0) var_dump("wat");
  return $r;
}

function main() :mixed{
  $x = vec[1, 2, 3];
  foo(inout $x, 24);
  foo(inout $x, 42, 'apple', 200);
  foo(inout $x, 8, 99, 'green');
  foo(inout $x, 16);
  var_dump($x);
  var_dump(bar(inout $x));
  var_dump(bar(inout $x));
  var_dump(bar(inout $x));
  var_dump(bar(inout $x));
  var_dump($x);
}


<<__EntryPoint>>
function main_variadic_inout() :mixed{
main();
}
