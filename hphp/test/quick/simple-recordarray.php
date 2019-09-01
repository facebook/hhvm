<?hh

final record Foo {
  x: int,
}

function foo(array $a) : array {
  $a['x'] = $a['x'] + 1;
  $a['y'] = 42;
  return $a;
}

<<__EntryPoint>>
function main() {
  $a = Foo@['x' => 10];
  $b = foo($a);
  var_dump(count($a));
  var_dump(count($b));
  var_dump($a['x']);
  var_dump($b['x']);
  var_dump($b['y']);
  try {
    var_dump($a['y']);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
