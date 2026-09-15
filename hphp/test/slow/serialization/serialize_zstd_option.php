<?hh

function check(mixed $expected): void {
  $actual = unserialize(
    serialize($expected, dict['zstd' => true]),
    dict['zstd' => true],
  );
  if (serialize($actual) !== serialize($expected)) {
    echo "round-trip mismatch for ".var_export($expected, true)."\n";
  }
}

class Priv { private $a = 1; protected $b = 2; public $c = 3; }
class Derived extends Priv { private $a = 10; public $d = 4; }
class Ser implements Serializable {
  public $x = 7;
  public function serialize(): mixed { return serialize(dict['k' => 'v']); }
  public function unserialize($s): mixed {}
}
class Node { public $next = null; public $val = 0; }

<<__EntryPoint>>
function main(): void {
  // Primitives
  check(null);
  check(true);
  check(false);
  check(0);
  check(-1);
  check(42);
  check(0.0);
  check(3.14);
  check(-2.5);
  check('');
  check('hello, world');
  check("with\0embedded\0nulls");

  // Containers
  check(dict[]);
  check(dict['hello' => 42, 'baz' => 100]);
  check(vec[]);
  check(vec[1, 2, 3, 4]);
  check(keyset[]);
  check(keyset['a', 'b', 'c']);
  check(shape('foo' => dict['1' => 2]));

  check(new Priv());
  check(new Derived());
  check(new Ser());
  check(Vector{1, 2, 3});
  check(Map{'a' => 1, 'b' => 2});
  check(Set{'x', 'y'});
  check(Pair{1, 'two'});

  // Reference edges: a self-referential object (R:) and a shared object (r:).
  $self = new Node(); $self->next = $self;
  check($self);
  $shared = new Node();
  check(vec[$shared, $shared]);

  $r = fopen(__DIR__.'/resource.txt', 'r');
  check(vec['a', $r, 'b']);

  $data = dict[];
  for ($i = 0; $i < 100000; $i++) {
    $data[$i] = 2 * $i - 1;
  }
  check($data);

  // A single huge (multi-MB) string value: the large-single-append path.
  check(str_repeat('abcdefghij', 800000));  // 8 MB string body

  // A string walked across the flush boundary at varied lengths.
  $base = str_repeat('a', 125000);
  for ($i = 0; $i < 200; $i++) {
    check(shape('foo' => $base.str_repeat('b', $i)));
  }

  echo "ok\n";
}
