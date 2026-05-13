<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Optional tuple entries: the 'optional' keyword marks an entry as nullable.

function test_optional_tuple_entry(
  (int, string, float) $t,
): void {
  tuple($a, optional $b, ...) = $t;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<?string>($b);
}

function test_optional_with_optional_type(
  (int, optional string) $t,
): void {
  // Both the type position (optional string) and the pattern (optional $b)
  // contribute to nullable. Regardless, $b should be ?string.
  tuple($a, optional $b) = $t;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<?string>($b);
}

function test_non_optional_entry(
  (int, string) $t,
): void {
  tuple($a, $b) = $t;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<string>($b);
}
