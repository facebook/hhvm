<?hh
class C<T> {}
class S<T> extends C<T> {
  public function __construct(private T $x) {}
  public function get() : T { return $this->x; }
}

function f(bool $b, dynamic $d, C<int> $c) : vec<int> {
  if ($b) {
    $x = $d;
  } else {
    $x = $c;
  }
  $x as S<_>;
  hh_show($x); // must not be S<int>
  $i = $x->get();
  hh_show($i); // must not be int
  return vec[$x->get()];
}

function expect_int(int $i) : void {}

<<__EntryPoint>>
function main() : void {
  $vi = f(true, new S<string>("1"), new C());
  expect_int($vi[0]);
}
