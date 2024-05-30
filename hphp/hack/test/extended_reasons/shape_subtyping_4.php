<?hh

type required_a_at_int = shape('a' => int);
type required_a_at_bool = shape('a' => bool);

function expects_required_a_at_int_alias(
 required_a_at_int $s,
): void {
}

function passes_required_a_at_bool_alias(
  required_a_at_bool $s,
): void {
  expects_required_a_at_int_alias($s);
}
