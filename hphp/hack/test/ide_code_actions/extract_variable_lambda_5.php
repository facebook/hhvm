<?hh

function map<T>(vec<T> $v, (function(T): T) $f): vec<T> {
  return vec[];
}

function main(): void {
  () ==> map(vec[], function($item) {
    $y = /*range-start*/$item + 1/*range-end*/;
    return $y;
  });
}
