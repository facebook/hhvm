<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Test destructuring with error types: mixed, nullable, nonnull

function test_mixed_shape(mixed $m): void {
  shape('x' => $x, ...) = $m; // error: mixed not indexable
}

function test_mixed_tuple(mixed $m): void {
  tuple($a, $b) = $m; // error: mixed not a tuple
}

function test_nullable_shape(): void {
  $s = null as ?shape('x' => int);
  shape('x' => $x) = $s; // error: nullable not destructurable
}

function test_nullable_tuple(): void {
  $t = null as ?(int, string);
  tuple($a, $b) = $t; // error: nullable not destructurable
}

function test_nonnull_shape(nonnull $x): void {
  shape('a' => $a, ...) = $x; // error: nonnull not a shape
}

function test_nonnull_tuple(nonnull $x): void {
  tuple($a, $b) = $x; // error: nonnull not a tuple
}

function test_int_shape(int $i): void {
  shape('x' => $x, ...) = $i; // error: int not a shape
}

function test_string_tuple(string $s): void {
  tuple($a) = $s; // error: string not a tuple
}

// Union of tuple + non-tuple: should error and not silently infer
// types from the tuple arm only.
class Foo {}
function test_tuple_union_non_tuple(bool $b): void {
  if ($b) {
    $u = tuple(1, 'hello');
  } else {
    $u = new Foo();
  }
  // $u : (int, string) | Foo -- should error
  tuple($a, $b2) = $u;
}
