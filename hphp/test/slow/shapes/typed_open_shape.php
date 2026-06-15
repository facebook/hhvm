<?hh
// Test that HHVM accepts typed open shape syntax in type hints.
// Typed open shapes use `type...` instead of bare `...` to constrain
// the type of unknown fields.

// ---- Function parameter and return type hints ----

function takes_typed_open(shape('a' => int, string...) $s): void {
  var_dump($s['a']);
}

function returns_typed_open(): shape('a' => int, string...) {
  return shape('a' => 1, 'b' => "hello");
}

// ---- No named fields, only typed unknown ----

function takes_only_typed_unknown(shape(string...) $s): void {
  var_dump($s);
}

// ---- Various unknown field types ----

function takes_int_unknown(shape('x' => string, int...) $s): void {
  var_dump($s['x']);
}

function takes_vec_unknown(shape('x' => int, vec<string>...) $s): void {
  var_dump($s['x']);
}

function takes_nullable_unknown(shape('x' => int, ?string...) $s): void {
  var_dump($s['x']);
}

function takes_dict_unknown(shape(dict<string, int>...) $s): void {
  var_dump($s);
}

// ---- Multiple named fields with typed open ----

function takes_multi_field(shape('a' => int, 'b' => bool, string...) $s): void {
  var_dump($s['a']);
  var_dump($s['b']);
}

// ---- Type alias ----

type MyTypedOpenShape = shape('name' => string, int...);

function takes_alias(MyTypedOpenShape $s): void {
  var_dump($s['name']);
}

// ---- Class property and method hints ----

class ShapeHolder {
  public shape('id' => int, string...) $data;

  public function __construct(shape('id' => int, string...) $data) {
    $this->data = $data;
  }

  public function getData(): shape('id' => int, string...) {
    return $this->data;
  }
}

// ---- Nested in generic types ----

function takes_vec_of_typed_open(vec<shape('a' => int, string...)> $v): void {
  foreach ($v as $s) {
    var_dump($s['a']);
  }
}

function takes_dict_of_typed_open(
  dict<string, shape('a' => int, string...)> $d,
): void {
  foreach ($d as $k => $s) {
    var_dump($k);
    var_dump($s['a']);
  }
}

<<__EntryPoint>>
function main(): void {
  echo "=== Parameter and return hints ===\n";
  takes_typed_open(shape('a' => 42, 'b' => "extra"));
  $r = returns_typed_open();
  var_dump($r['a']);
  var_dump($r['b']);

  echo "\n=== Only typed unknown ===\n";
  takes_only_typed_unknown(shape('x' => "hello", 'y' => "world"));

  echo "\n=== Various unknown field types ===\n";
  takes_int_unknown(shape('x' => "test", 'y' => 99));
  takes_vec_unknown(shape('x' => 1, 'extra' => vec["a", "b"]));
  takes_nullable_unknown(shape('x' => 5, 'z' => null));
  takes_dict_unknown(shape('k' => dict["a" => 1]));

  echo "\n=== Multiple named fields ===\n";
  takes_multi_field(shape('a' => 10, 'b' => true, 'c' => "extra"));

  echo "\n=== Type alias ===\n";
  takes_alias(shape('name' => "Alice", 'score' => 100));

  echo "\n=== Class property and method ===\n";
  $holder = new ShapeHolder(shape('id' => 1, 'label' => "test"));
  $data = $holder->getData();
  var_dump($data['id']);
  var_dump($data['label']);

  echo "\n=== Nested in generics ===\n";
  takes_vec_of_typed_open(vec[shape('a' => 1), shape('a' => 2, 'b' => "x")]);
  takes_dict_of_typed_open(dict[
    'first' => shape('a' => 10),
    'second' => shape('a' => 20, 'extra' => "y"),
  ]);

  echo "\nDone.\n";
}
