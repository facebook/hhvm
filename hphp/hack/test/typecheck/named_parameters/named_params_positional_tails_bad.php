<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function incompatible_name(
  (function(named int $x, int...): int) $f,
): (function(named string $x, int...): int) {
  return $f;
}

function incompatible_variadic_subtype(
  (function(named string $x, int...): int) $f,
): (function(num, named string $x): int) {
  return $f;
}

function incompatible_variadic_supertype(
  (function(optional int, named string $x, num...): int) $f,
): (function(named string $x, num...): int) {
  return $f;
}

function incompatible_splat_subtype(
  (function(named string $x, ...(int)): int) $f,
): (function(num, named string $x): int) {
  return $f;
}

function incompatible_splat_supertype(
  (function(int, named string $x): int) $f,
): (function(named string $x, ...(num)): int) {
  return $f;
}

function missing_positionals(
  (function(named string $x): int) $f,
): (function(int, named string $x, ...(bool)): int) {
  return $f;
}
