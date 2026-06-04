<?hh

class RC {
  public int $id;
  public function __construct(int $id) { $this->id = $id; }
}

// Exercises DecRef destroy path: refcount reaches zero, object freed.
function test_destroy(): void {
  $a = new RC(1);
  $b = vec[$a];
  $a = null;
  $b = null;
}

// Exercises DecRef survive path: refcount decremented but object lives.
function test_survive(): int {
  $a = new RC(2);
  $b = $a;
  $c = $a;
  $b = null;
  $c = null;
  return $a->id;
}

// Exercises DecRef of vec elements (multiple decrefs in sequence).
function test_collection(): int {
  $objs = vec[new RC(10), new RC(20), new RC(30)];
  $ref = $objs[1];
  $sum = 0;
  foreach ($objs as $o) {
    $sum += $o->id;
  }
  $objs = null;
  $sum += $ref->id;
  return $sum;
}

// Exercises generic DecRef path (mixed type forces generic stub).
function test_generic(mixed $val): mixed {
  $local = $val;
  $copy = $local;
  $local = null;
  return $copy;
}

<<__EntryPoint>>
function main(): void {
  for ($i = 0; $i < 5000; $i++) {
    test_destroy();

    $id = test_survive();
    if ($id !== 2) { echo "FAIL survive: $id\n"; return; }

    $sum = test_collection();
    if ($sum !== 80) { echo "FAIL collection: $sum\n"; return; }

    $g1 = test_generic(new RC(99));
    if (!($g1 is RC) || $g1->id !== 99) { echo "FAIL generic obj\n"; return; }

    $g2 = test_generic(vec[1, 2, 3]);
    if ($g2 !== vec[1, 2, 3]) { echo "FAIL generic vec\n"; return; }

    $g3 = test_generic("hello");
    if ($g3 !== "hello") { echo "FAIL generic str\n"; return; }

    $g4 = test_generic(42);
    if ($g4 !== 42) { echo "FAIL generic int\n"; return; }
  }
  echo "PASSED\n";
}
