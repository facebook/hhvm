<?hh

function g(float $x, varray<int> $y, int ...$ys): void {}

function call_unpack((int, float, int, float, int, float) $packed,):void {
  /* HH_FIXME[4110] */
  g(...$packed);
}
