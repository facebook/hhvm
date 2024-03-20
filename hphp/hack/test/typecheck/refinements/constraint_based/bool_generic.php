<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type CT = bool | int;

function f<T as CT>(T $ct): void {
  if ($ct is bool) {
    hh_expect<bool>($ct);
    hh_expect<T>($ct);
  } else {
    hh_expect<int>($ct);
    hh_expect<T>($ct);
  }
}
