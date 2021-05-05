<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I {
  abstract const type Ta as nothing;
}
function oops0<T as I, Ta>():(function(Ta): nothing)
  where T::Ta = Ta {
  return $x ==> $x;
}
  function oops1<T as I, Ta>():(function(mixed): Ta)
  where T::Ta = Ta, Ta super mixed {
  return $x ==> $x;
}
  function oops():(function(mixed): nothing) {
    return $x ==> {
      $f0 = oops0();
      $f1 = oops1();
      return $f0($f1($x));
    };
}
<<__EntryPoint>>
function breakit(): int {
  $f = oops();
  return $f('hi');
}
