<?hh

function takes_int(int $x): void {}
function untyped() { return null; }

function test(vec<int> $v): void {
  $x = $v[0] > 0 ? $v[0] : untyped();
  if ($x !== null) {
    takes_int($x);
  }
}
