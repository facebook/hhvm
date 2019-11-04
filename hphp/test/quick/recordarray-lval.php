<?hh

record Foo {
  x: vec,
}

function foo(array $a) : array {
  $a['x'][0] = 11;
  $a['y'][0] = 52;
  $a[1][0] = 62;
  return $a;
}

<<__EntryPoint>>
function main() {
  $a = Foo@['x' => vec[10, 42]];
  $b = foo($a);
  var_dump($a['x']);
  var_dump($b['x']);
  var_dump($b['y']);
  var_dump($b[1]);
}
