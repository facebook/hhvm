<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type CT = bool | int;

function f<T as CT>(bool $b, T $ct): void {
  if ($b && !$ct is bool) {
    hh_expect<int>($ct);
    hh_expect<T>($ct);
  }
  if (!$ct is bool && $b) {
    hh_expect<int>($ct);
    hh_expect<T>($ct);
  }
}

function g<T as CT>(bool $b, T $ct): void {
  if ($ct is bool || $b) {
    ;
  } else {
    hh_expect<int>($ct);
    hh_expect<T>($ct);
  }
  if ($b || $ct is bool) {
    ;
  } else {
    hh_expect<int>($ct);
    hh_expect<T>($ct);
  }
}

case type CTBoolIntVec = bool | int | vec<int>;

function boolIntVec<T as CTBoolIntVec>(bool $b, T $ct): void {
  if ($ct is bool || $ct is int) {
    hh_expect<T>($ct);
  } else {
    hh_expect<vec<int>>($ct);
    hh_expect<T>($ct);
  }
}
