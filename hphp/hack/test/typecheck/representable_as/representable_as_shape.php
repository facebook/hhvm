<?hh
<<file: __EnableUnstableFeatures('representable_as')>>

function takes_representable_as_dict_arraykey_int(\HH\Runtime\RepresentableAs<dict<arraykey, int>> $c): void {}
function takes_representable_as_dict_arraykey_num(\HH\Runtime\RepresentableAs<dict<arraykey, num>> $c): void {}
function takes_representable_as_dict_arraykey_arraykey(\HH\Runtime\RepresentableAs<dict<arraykey, arraykey>> $c): void {}
function takes_representable_as_dict_string_int(\HH\Runtime\RepresentableAs<dict<string, int>> $c): void {}
function takes_representable_as_dict_int_int(\HH\Runtime\RepresentableAs<dict<int, int>> $c): void {}

function test_shape_int_int(shape('a' => int, 'b' => int) $s): void {
  // shape('a' => int, 'b' => int) <: RepresentableAs<dict<arraykey, int>> — should pass
  takes_representable_as_dict_arraykey_int($s);
}

function test_shape_int_float(shape('a' => int, 'b' => float) $s): void {
  // shape('a' => int, 'b' => float) <: RepresentableAs<dict<arraykey, num>> — should pass (int|float <: num)
  takes_representable_as_dict_arraykey_num($s);
}

function test_shape_int_string(shape('a' => int, 'b' => string) $s): void {
  // shape('a' => int, 'b' => string) <: RepresentableAs<dict<arraykey, arraykey>> — should pass (int|string <: arraykey)
  takes_representable_as_dict_arraykey_arraykey($s);
}

function test_shape_int_string_not_dict_int(shape('a' => int, 'b' => string) $s): void {
  // shape('a' => int, 'b' => string) <: RepresentableAs<dict<arraykey, int>> — should FAIL
  takes_representable_as_dict_arraykey_int($s);
}

function test_closed_shape_dict_string(shape('a' => int, 'b' => int) $s): void {
  // Closed shape keys are strings, so dict<string, int> — should pass
  takes_representable_as_dict_string_int($s);
}

function test_closed_shape_dict_int_fails(shape('a' => int, 'b' => int) $s): void {
  // Closed shape keys are strings, not ints — should FAIL
  takes_representable_as_dict_int_int($s);
}

function test_open_shape_dict_string_fails(shape('a' => int, ...) $s): void {
  // Open shape keys are arraykey, not string — should FAIL
  takes_representable_as_dict_string_int($s);
}

function test_open_shape_dict_arraykey_mixed(shape('a' => int, ...) $s): void {
  // Open shape unknown values are mixed, so value union is int|mixed = mixed
  // dict<arraykey, mixed> <: dict<arraykey, int> — should FAIL
  takes_representable_as_dict_arraykey_int($s);
}

function test_open_shape_dict_arraykey_mixed_pass(\HH\Runtime\RepresentableAs<dict<arraykey, mixed>> $c): void {}
function test_open_shape_dict_arraykey_mixed_ok(shape('a' => int, ...) $s): void {
  // Open shape: dict<arraykey, int|mixed> = dict<arraykey, mixed> — should pass
  test_open_shape_dict_arraykey_mixed_pass($s);
}

function test_open_shape_dict_arraykey_num_pass(\HH\Runtime\RepresentableAs<dict<arraykey, num>> $c): void {}
function test_open_shape_dict_arraykey_string_ok(shape('a' => int, float...) $s):void {
  test_open_shape_dict_arraykey_num_pass($s);
  // Should FAIL
  takes_representable_as_dict_arraykey_int($s);
}

enum E: string as string {
  A = 'a';
}

function test_class_const_key_string_enum(shape(E::A => int) $s): void {
  // E::A resolves to enum type E, which has `as string` constraint — should pass
  takes_representable_as_dict_string_int($s);
}

function test_class_const_key_arraykey(shape(E::A => int) $s): void {
  // Class const key — key type is E <: arraykey — should pass
  takes_representable_as_dict_arraykey_int($s);
}

enum F: int as int {
  X = 1;
}

function test_int_enum_const_key_dict_int(shape(F::X => int) $s): void {
  // F::X resolves to enum type F, which has `as int` constraint — should pass
  takes_representable_as_dict_int_int($s);
}

function test_int_enum_const_key_dict_arraykey(shape(F::X => int) $s): void {
  // F <: arraykey — should pass
  takes_representable_as_dict_arraykey_int($s);
}

function test_int_enum_const_key_dict_string_fails(shape(F::X => int) $s): void {
  // F::X resolves to enum type F with `as int`, int is not string — should FAIL
  takes_representable_as_dict_string_int($s);
}

// -- Traversable / KeyedTraversable tests --
// shape upcast produces dict<K, V>, which implements KeyedTraversable<K, V>
// and Traversable<V>, so these should work transitively.

function takes_representable_as_traversable_int(\HH\Runtime\RepresentableAs<Traversable<int>> $c): void {}
function takes_representable_as_traversable_num(\HH\Runtime\RepresentableAs<Traversable<num>> $c): void {}
function takes_representable_as_keyed_traversable_string_int(\HH\Runtime\RepresentableAs<KeyedTraversable<string, int>> $c): void {}
function takes_representable_as_keyed_traversable_arraykey_int(\HH\Runtime\RepresentableAs<KeyedTraversable<arraykey, int>> $c): void {}

function test_closed_shape_traversable_int(shape('a' => int, 'b' => int) $s): void {
  // dict<string, int> <: Traversable<int> — should pass
  takes_representable_as_traversable_int($s);
}

function test_closed_shape_traversable_num(shape('a' => int, 'b' => float) $s): void {
  // dict<string, int|float> <: Traversable<num> — should pass (int|float <: num)
  takes_representable_as_traversable_num($s);
}

function test_closed_shape_keyed_traversable_string_int(shape('a' => int, 'b' => int) $s): void {
  // dict<string, int> <: KeyedTraversable<string, int> — should pass
  takes_representable_as_keyed_traversable_string_int($s);
}

function test_closed_shape_keyed_traversable_arraykey_int(shape('a' => int, 'b' => int) $s): void {
  // dict<string, int> <: KeyedTraversable<arraykey, int> — should pass (string <: arraykey)
  takes_representable_as_keyed_traversable_arraykey_int($s);
}

function test_closed_shape_traversable_int_fails(shape('a' => int, 'b' => string) $s): void {
  // dict<string, int|string> <: Traversable<int> — should FAIL
  takes_representable_as_traversable_int($s);
}

function test_closed_shape_keyed_traversable_string_int_fails(shape('a' => int, 'b' => string) $s): void {
  // dict<string, int|string> <: KeyedTraversable<string, int> — should FAIL
  takes_representable_as_keyed_traversable_string_int($s);
}
