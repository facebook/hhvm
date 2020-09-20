<?hh

record Foo {
  vec x;
}

function foo(Foo $aa) : Foo {
  $bb = $aa;
  $bb['x'][0] = 42;
  return $bb;
}

<<__EntryPoint>>
function main() {
  $a = Foo['x' => vec[10, 20]];
  $b = foo($a);
  var_dump($a['x']);
  var_dump($b['x']);
}
