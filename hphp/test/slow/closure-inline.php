<?hh

class F {
  public int $x;
  public int $y;
}

<<__NEVER_INLINE>>
function foo(int $x, F $y) {
  $z = $y->y + $x;
  return (() ==> $x + $y->x + $z)();
}

<<__EntryPoint>>
function main() {
  $q = new F;
  $q->x = 10;
  $q->y = 20;
  var_dump(foo(5, $q));
  var_dump(foo(10, $q));
  var_dump(foo(15, $q));
}
