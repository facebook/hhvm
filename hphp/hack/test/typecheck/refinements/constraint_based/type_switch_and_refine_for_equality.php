<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type CT = bool | int;

function f<T as CT>(bool $b, T $ct1, T $ct2): void {
  if ($ct1 is bool && $ct1 === $ct2) {
    hh_expect<bool>($ct2);
  }
}
