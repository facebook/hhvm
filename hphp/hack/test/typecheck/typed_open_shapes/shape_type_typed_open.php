<?hh
// strict

// Tests for typed open shapes: shape('a' => int, string...)
// The key behavioral difference from bare `...` is that unknown fields
// are constrained to the specified type instead of `mixed`.

// ---- Shapes::idx returns the typed unknown value ----

// With bare `...`, Shapes::idx on unknown field returns `?mixed`.
// With `string...`, Shapes::idx on unknown field returns `?string`.
function idx_returns_typed_value(shape('a' => int, string...) $s): void {
  $x = Shapes::idx($s, 'unknown_field');
  hh_expect_equivalent<?string>($x);
}

// With `int...`, Shapes::idx on unknown field returns `?int`.
function idx_returns_int(shape('a' => string, int...) $s): void {
  $x = Shapes::idx($s, 'unknown_field');
  hh_expect_equivalent<?int>($x);
}

// Known fields still return their declared type, not the unknown type.
function idx_known_field(shape('a' => int, string...) $s): void {
  $x = Shapes::idx($s, 'a');
  hh_expect_equivalent<?int>($x);
}

// ---- Subtyping with extra fields ----

// A closed shape with string extra fields is a subtype of string... open shape.
function accept_string_open(shape('a' => int, string...) $_s): void {}

function closed_string_to_string_open(): void {
  $s = shape('a' => 1, 'b' => "hello", 'c' => "world");
  accept_string_open($s);
}

// A closed shape with no extra fields is a subtype of any typed open shape.
function closed_to_typed_open(): void {
  $s = shape('a' => 1);
  accept_string_open($s);
}

// ---- Complex unknown field types ----

function idx_vec(shape('a' => int, vec<string>...) $s): void {
  $x = Shapes::idx($s, 'unknown_field');
  hh_expect_equivalent<?vec<string>>($x);
}

function idx_nullable(shape('a' => int, ?string...) $s): void {
  $x = Shapes::idx($s, 'unknown_field');
  // ?string... means the unknown value type is ?string,
  // and Shapes::idx adds another layer of nullability -> ?(?string)
  // In Hack, ?(?string) simplifies to ?string
  hh_expect_equivalent<?string>($x);
}

// ---- No named fields, only typed unknown ----

function only_typed_unknown(shape(string...) $s): void {
  $x = Shapes::idx($s, 'anything');
  hh_expect_equivalent<?string>($x);
}

// ---- Return types ----

function return_typed_open(): shape('a' => int, string...) {
  return shape('a' => 1, 'b' => "hello");
}

// ---- Multiple named fields with typed open ----

function multi_field_idx(shape('a' => int, 'b' => bool, string...) $s): void {
  hh_expect<int>($s['a']);
  hh_expect<bool>($s['b']);
  $x = Shapes::idx($s, 'c');
  hh_expect_equivalent<?string>($x);
}
