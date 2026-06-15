<?hh
// strict

// Tests for variadic function parameters whose type is a shape.
// Ensures that closed shapes, open shapes, and typed open shapes
// all parse and typecheck correctly when used as variadic params.

// ---- Closed shape as variadic param ----

function variadic_closed_shape(shape('a' => int) ...$shapes): void {
  foreach ($shapes as $s) {
    hh_expect<int>($s['a']);
  }
}

function call_variadic_closed_shape(): void {
  variadic_closed_shape(shape('a' => 1), shape('a' => 2));
}

// ---- Open shape as variadic param ----

function variadic_open_shape(shape('a' => int, ...) ...$shapes): void {
  foreach ($shapes as $s) {
    hh_expect<int>($s['a']);
  }
}

function call_variadic_open_shape(): void {
  variadic_open_shape(shape('a' => 1, 'b' => "extra"), shape('a' => 2));
}

// ---- Typed open shape as variadic param ----

function variadic_typed_open_shape(shape('a' => int, string...) ...$shapes): void {
  foreach ($shapes as $s) {
    hh_expect<int>($s['a']);
    $x = Shapes::idx($s, 'unknown');
    hh_expect_equivalent<?string>($x);
  }
}

function call_variadic_typed_open_shape(): void {
  variadic_typed_open_shape(
    shape('a' => 1, 'b' => "hello"),
    shape('a' => 2, 'c' => "world"),
  );
}

// ---- Typed open shape with no named fields as variadic param ----

function variadic_typed_open_only(shape(string...) ...$shapes): void {
  foreach ($shapes as $s) {
    $x = Shapes::idx($s, 'any_key');
    hh_expect_equivalent<?string>($x);
  }
}

function call_variadic_typed_open_only(): void {
  variadic_typed_open_only(
    shape('x' => "hello"),
    shape('y' => "world", 'z' => "!"),
  );
}

// ---- Complex typed open shape as variadic param ----

function variadic_complex_typed_open(shape('id' => int, vec<string>...) ...$shapes): void {
  foreach ($shapes as $s) {
    hh_expect<int>($s['id']);
    $x = Shapes::idx($s, 'tags');
    hh_expect_equivalent<?vec<string>>($x);
  }
}

function call_variadic_complex_typed_open(): void {
  variadic_complex_typed_open(
    shape('id' => 1, 'tags' => vec["a", "b"]),
    shape('id' => 2, 'labels' => vec["c"]),
  );
}
