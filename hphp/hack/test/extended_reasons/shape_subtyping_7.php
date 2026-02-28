<?hh

type required_a_at_int = shape('a' => int);
type missing_a = shape('b' => int);

function expects_required_a_at_int_alias(
  required_a_at_int $s,
): void {
}

function passes_missing_a_alias(
  missing_a $s,
): void {
  expects_required_a_at_int_alias($s);
}
