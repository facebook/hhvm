<?hh

function test_iterator(ArrayIterator $it, mixed $mutate): void {
  print("  Basic iteration:\n");
  foreach ($it as $k => $v) {
    $k = json_encode($k);
    $v = json_encode($v);
    print("  $k => $v\n");
  }

  $mutate();

  print("  Basic iteration (again):\n");
  foreach ($it as $k => $v) {
    $k = json_encode($k);
    $v = json_encode($v);
    print("    $k => $v\n");
  }

  $it->rewind();
  print("  Iteration by hand:\n");
  for ($i = 0; $i < 7; $i++) {
    list($k, $v, $valid) = tuple($it->key(), $it->current(), $it->valid());
    $k = json_encode($k);
    $v = json_encode($v);
    $valid = json_encode($valid);
    print("    $k => $v (\$valid = $valid)\n");
    $it->next();
  }
}

class C {
  public mixed $a = null;
  public bool $b = false;
  public int $c = 17;
  public int $d = 0;
  public string $e = '51';
  public ?int $f = null;
}

<<__EntryPoint>>
function main(): void {
  print("\nTesting varray base:\n");
  $base = vec[null, false, 17, 34, '51'];
  $it = new ArrayIterator($base);
  test_iterator($it, () ==> { $base[] = 68; });

  print("\nTesting darray base:\n");
  $base = dict['a' => null, 'b' => false, '' => 17, 0 => 34, 'e' => '51'];
  $it = new ArrayIterator($base);
  test_iterator($it, () ==> { $base['f'] = 68; });

  print("\nTesting object base:\n");
  $base = new C();
  $it = new ArrayIterator($base);
  test_iterator($it, () ==> { $base->f = 68; });
}
