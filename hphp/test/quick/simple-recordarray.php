<?hh

record Foo {
  int x;
}

class Bar {
  public int $f = 42;
}

record FooBar {
  Bar z;
}

function foo(array $a) : array {
  $a['x'] = $a['x'] + 1;
  unset($a[0]); // no-op
  $a['y'] = 42;
  return $a;
}

function bar(array $x) : array {
  $x[0] = 46;
  return $x;
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

  try {
    var_dump($a[0]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  $a[0] = 43;
  // No more notice for $a
  $a[1] = 44;
  var_dump($a[0]);
  var_dump($a[1]);
  var_dump($a['x']);

  $c = bar($b);
  var_dump($c);
  try {
    var_dump($b[0]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  $o1 = new Bar;
  $o2 = new Bar;
  $o2->f = 10;
  $r1 = FooBar@['z' => $o1];
  $r2 = $r1;
  $r1['z'] = $o2;
  var_dump($r1['z']);
  var_dump($r2['z']);
}
