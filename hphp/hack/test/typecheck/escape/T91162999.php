<?hh // strict

class Box<T> {
  public function __construct(public T $data) {}
};

<<__EntryPoint>>
function f(): string {
  $a = 42;
  $v = Vector{};
  $w = "";
  $f = (mixed $x) ==> {
    invariant($x is Box<_>, "");
    $v[] = $x->data;
    $x->data = $v[0];
  };
  $f(new Box(42));
  $sbox = new Box("oops");
  $f($sbox);
  return $sbox->data;
}
