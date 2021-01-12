<?hh

function vec_foreach<Te>(
  vec<int> $v,
  (function (int)[Te]: int) $f,
)[Te]: void {
  foreach ($v as $x) {
    $f($x);
  }
}

function empty_vec_factory<Te>()[
]: (function ()[Te]: vec<int>) {
  return ()[] ==> vec[];
}
