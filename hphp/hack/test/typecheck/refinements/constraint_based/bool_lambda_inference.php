<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type CT = bool | int;

function iter<T>(vec<T> $vec, (function(T): void) $f): void {
  foreach ($vec as $v) {
    $f($v);
  }
}

function test(vec<CT> $vec): void {
  iter(
    $vec,
    $x ==> {
      if ($x is bool) {
        hh_expect<bool>($x);
      } else {
        hh_expect<int>($x);
      }
    },
  );
}
