<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

type NamedVariadic = (function(named string $x, int...): int);
newtype WrappedNamedVariadic as NamedVariadic = NamedVariadic;

function reflexive(
  (function(named string $x, int...): int) $f,
): (function(named string $x, int...): int) {
  return $f;
}

function variadic_subtype(
  (function(named string $x, num...): int) $f,
): (function(int, named string $x): int) {
  return $f;
}

function variadic_supertype(
  (function(optional int, optional named string $x, int...): int) $f,
): (function(named string $x, int...): int) {
  return $f;
}

function contravariant_reordered_names(
  (function(optional named arraykey $y, named ?string $x, num...): int) $f,
): (function(named string $x, named int $y, int...): int) {
  return $f;
}

function splat_subtype(
  (function(named string $x, ...(int)): int) $f,
): (function(int, named string $x): int) {
  return $f;
}

function splat_supertype(
  (function(int, named string $x, optional bool): int) $f,
): (function(named string $x, ...(int, optional bool)): int) {
  return $f;
}
