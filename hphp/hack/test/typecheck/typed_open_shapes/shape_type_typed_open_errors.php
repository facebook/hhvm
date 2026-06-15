<?hh
// strict

// Error cases for typed open shapes.
// These tests verify that the unknown field type constraint is enforced.

function accept_string_open(shape('a' => int, string...) $_s): void {}
function accept_int_open(shape('a' => int, int...) $_s): void {}

// ---- Subtyping errors: wrong extra field type ----

// Extra field is int, but string... requires string.
function pass_int_to_string_open(): void {
  $s = shape('a' => 1, 'b' => 42);
  accept_string_open($s);
}

// Extra field is string, but int... requires int.
function pass_string_to_int_open(): void {
  $s = shape('a' => 1, 'b' => "hello");
  accept_int_open($s);
}

// ---- Return type errors ----

// Returning a shape with int extra field where string... is expected.
function return_wrong_unknown_type(): shape('a' => int, string...) {
  return shape('a' => 1, 'b' => 42);
}

// ---- Shapes::idx type mismatch ----

// Shapes::idx on string... open shape returns ?string, not ?int.
function idx_type_mismatch(shape('a' => int, string...) $s): void {
  $x = Shapes::idx($s, 'unknown_field');
  hh_expect_equivalent<?int>($x);
}
