<?hh

function expects_required_a_at_int(
shape('a' => int) $s,
): void {
}

function passes_required_a_at_bool(
  shape('a' => bool) $s,
): void {
  expects_required_a_at_int($s);
}

function passes_optional_a_at_int(
  shape(?'a' => int) $s,
): void {
  expects_required_a_at_int($s);
}

function passes_optional_a_at_bool(
  shape(?'a' => bool) $s,
): void {
  expects_required_a_at_int($s);
}


type required_a_at_int = shape('a' => int);
type required_a_at_bool = shape('a' => bool);
type optional_a_at_int = shape(?'a' => int);
type optional_a_at_bool = shape(?'a' => bool);
type missing_a = shape('b' => int);

function expects_required_a_at_int_alias(
required_a_at_int $s,
): void {
}

function passes_required_a_at_bool_alias(
  required_a_at_bool $s,
): void {
  expects_required_a_at_int_alias($s);
}

function passes_optional_a_at_int_alias(
  optional_a_at_int $s,
): void {
  expects_required_a_at_int_alias($s);
}

function passes_optional_a_at_bool_alias(
  optional_a_at_bool $s,
): void {
  expects_required_a_at_int_alias($s);
}


function passes_missing_a_alias(
  missing_a $s,
): void {
  expects_required_a_at_int_alias($s);
}
