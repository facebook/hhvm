<?hh

function foo(): void {
  $v = $vec["a", "b"];
  list($a, /*range-start*//*range-end*/$b) = $v;
}
