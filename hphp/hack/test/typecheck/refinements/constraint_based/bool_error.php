<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type CT = bool | float;

function g<T as CT>(T $n): int {
  if ($n is bool) {
    return (int)$n;
  } else {
    // $n: T & float
    return $n; // Errors
  }
}
