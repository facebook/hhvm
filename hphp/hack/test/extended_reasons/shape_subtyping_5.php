<?hh

type required_a_at_int = shape('a' => int);
type optional_a_at_int = shape(?'a' => int);

function expects_required_a_at_int_alias(
required_a_at_int $s,
): void {
}

function passes_optional_a_at_int_alias(
  optional_a_at_int $s,
): void {
  expects_required_a_at_int_alias($s);
}
