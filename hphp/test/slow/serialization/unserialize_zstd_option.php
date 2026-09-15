<?hh

// Round-trip values through serialize() and unserialize() with zstd enabled.

class Point {
  public int $x = 0;
  public int $y = 0;
  public function __construct(int $x, int $y) { $this->x = $x; $this->y = $y; }
}

class Priv { private $a = 1; protected $b = 2; public $c = 3; }
class Derived extends Priv { private $a = 10; public $d = 4; }
class Ser implements Serializable {
  public $x = 7;
  public function serialize(): mixed { return serialize(dict['k' => 'v']); }
  public function unserialize($s): mixed { $this->x = unserialize($s)['k']; }
}
class Node { public $next = null; public $val = 0; }
class Wake { public $prop = 'asleep'; function __wakeup(): void { $this->prop = 'awake'; } }
class Box<reify T> { public function __construct(public int $val = 0) {} }

function check(mixed $expected): void {
  $actual = unserialize(
    serialize($expected, dict['zstd' => true]),
    dict['zstd' => true],
  );
  if (serialize($actual) !== serialize($expected)) {
    echo "round-trip mismatch for ".var_export($expected, true)."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  // Primitives, including the double special-cases the parser branches on.
  check(null);
  check(true);
  check(false);
  check(0);
  check(-1);
  check(42);
  check(0.0);
  check(3.14);
  check(-2.5);
  check(INF);
  check(-INF);
  check(NAN);

  // Strings, including embedded nulls and the empty string.
  check('');
  check('hello, world');
  check("with\0embedded\0nulls");

  // Containers and nested shapes.
  check(dict[]);
  check(dict['hello' => 42, 'baz' => 100]);
  check(vec[]);
  check(vec[1, 2, 3, 4]);
  check(keyset[]);
  check(keyset['a', 'b', 'c']);
  check(shape('foo' => dict['1' => 2], 'bar' => vec[1.5, 2.5]));

  // Objects exercise the 'O' path (property matchString, which the streaming
  // source deliberately falls back from instead of using the fast rewind).
  check(new Point(3, 4));
  check(vec[new Point(1, 2), new Point(-5, -6)]);

  // Private/protected mangled prop names, inheritance, and the Serializable
  // interface (its unserialize() hook runs on the streaming result too).
  check(new Priv());
  check(new Derived());
  check(new Ser());

  // Reference edges: a self-referential object (R:) and a shared object (r:).
  // The streaming source must resolve back-references correctly even though it
  // has no random back-seek into the compressed window.
  $self = new Node(); $self->next = $self; $self->val = 9;
  check($self);
  $shared = new Node();
  check(vec[$shared, $shared]);
  // The shared reference must stay shared after the streaming round-trip.
  $rt = unserialize(
    serialize(vec[$shared, $shared], dict['zstd' => true]),
    dict['zstd' => true],
  );
  if ($rt[0] !== $rt[1]) {
    echo "shared reference not preserved by streaming unserialize\n";
  }

  // Reified-generic class: its serialized form carries a leading
  // 86reified_prop that the streaming source must consume via the normal
  // primitives (matchString's contiguous fast path is unavailable).
  check(new Box<int>(42));
  check(vec[new Box<int>(1), new Box<int>(2)]);

  // __wakeup runs on the streaming result.
  $w = unserialize(
    serialize(new Wake(), dict['zstd' => true]),
    dict['zstd' => true],
  );
  if ($w->prop !== 'awake') {
    echo "__wakeup did not run on streaming result\n";
  }

  // A large structure that spans multiple 128 KiB decompression windows: the
  // streaming source must compact/refill correctly across value boundaries.
  $data = dict[];
  for ($i = 0; $i < 100000; $i++) {
    $data[(string)$i] = $i % 2 === 0 ? $i : "v".$i;
  }
  check($data);

  // A single multi-MB string value read out of the streaming window in chunks.
  check(str_repeat('abcdefghij', 800000));

  // Strings straddling the flush boundary at varied lengths (the readStr window
  // may need to grow/compact mid-value).
  $base = str_repeat('a', 125000);
  for ($i = 0; $i < 64; $i++) {
    check(shape('s' => $base.str_repeat('b', $i), 'n' => $i));
  }

  // A big HH\Map<string,int> — over the contiguous fast-path threshold. The
  // streaming source has no fast path, so this verifies the fallback matches.
  $m = new Map();
  for ($i = 0; $i < 5000; $i++) {
    $m["key".$i] = $i;
  }
  check($m);

  echo "ok\n";
}
