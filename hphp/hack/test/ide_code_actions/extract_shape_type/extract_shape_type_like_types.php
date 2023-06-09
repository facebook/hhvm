<?hh

function foo(vec<shape('a' => int)> $v): void {
  $sh = $v[0];
  // This test may fail if we start using the tast under Sound Dynamic (https://fburl.com/code/9lkro66x).
  // If that happens, then try stripping "like types" (`~shape()` to `shape()`)
  /*range-start*/$sh/*range-end*/;
}
