<?hh

function unpack_a(int $x, int ...$xs): void {}
function unpack_b(float $x, varray<int> $y, int ...$ys): void {}
function unpack_c(float $x, (float, int) $y, int ...$ys): void {}
function unpack_d(int $x, shape('u' => bool, 'v' => bool) $y): void {}
function unpack_e(
float $x,
(function(int, float): bool) $f,
int $y,
): void {}

function call_unpack(
Pair<int, float> $a,
(int, float, int, float, int, float) $b,
(float, varray<float>, float) $c,
(float, varray<int>, float, int) $d,
(float, int, float, float) $e,
shape('a' => float, 'b' => int, 'c' => int) $f,
(float, (float, float), int) $g,
(int, shape('u' => float, 'v' => int)) $h,
(float, (function(float, int): int), int) $i,
(float, (function(int, float): bool), int) $j,
): void {
  /* HH_FIXME[4110] */
  unpack_a(...$a);
  /* HH_FIXME[4110] */
  unpack_a(...$b);
  /* HH_FIXME[4110] */
  unpack_b(...$c);
  /* HH_FIXME[4110] */
  unpack_b(...$d);
  /* HH_FIXME[4110] */
  unpack_b(...$e);
  /* HH_FIXME[4110] */
  unpack_b(...$f);
  /* HH_FIXME[4110] */
  unpack_c(...$g);
  /* HH_FIXME[4110] */
  unpack_d(...$h);
  /* HH_FIXME[4110] */
  unpack_e(...$i);
}
