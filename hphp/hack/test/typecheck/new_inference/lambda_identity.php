<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function makeFunction<T1,T2>((function(T1):T2) $f):(function(T1):T2) {
  return $f;
}
function expectArraykey(arraykey $x):void { }
function testit(bool $b):(function(arraykey):arraykey) {
  if ($b) {
    // Ideally this should have a *generic* function type
    // But we don't support that, so the best we can do
    // is function(int|string):(int|string)
    $f = $x ==> $x;
    $a = $f(3);
    $b = $f("a");
    expectArraykey($a);
    expectArraykey($b);
    return $f;
  } else {
    // Force constraint-based checking by giving it a context
    $f = makeFunction($x ==> $x);
    $a = $f(3);
    $b = $f("a");
    expectArraykey($a);
    expectArraykey($b);
    return $f;
  }
}
