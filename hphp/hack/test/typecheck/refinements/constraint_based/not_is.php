<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type CT = bool | int;

// inverse of hphp/hack/test/typecheck/refinements/constraint_based/bool_generic.php
function f<T as CT>(T $ct): void {
  if (!$ct is bool) {
    hh_expect<int>($ct);
    hh_expect<T>($ct);
  } else {
    hh_expect<bool>($ct);
    hh_expect<T>($ct);
  }
}
