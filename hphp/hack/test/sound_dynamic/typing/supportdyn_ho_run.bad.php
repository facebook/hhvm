<?hh

class C {}

function f(
  ~vec<int> $x,
  ~vec<string> $y,
  vec<string> $y2,
  ~vec<C> $c,
  supportdyn<(function(vec<int>, vec<string>): bool)> $g,
  supportdyn<(function(vec<C>, vec<string>): bool)> $h,
  supportdyn<mixed> $m,
): void {
  $g($x, $x);
  $g($x);
  $h($c, $y);
  $h($c, $y2);
  $m($c);
}
