<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Tuple destructuring follows tuple-type ordering rules:
// after an optional entry, all following entries must also be optional.
function test_nontrailing_optional(
  (int, string) $t,
): void {
  tuple(optional $a, $b) = $t;
}
