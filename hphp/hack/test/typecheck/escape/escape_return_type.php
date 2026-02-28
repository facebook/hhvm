<?hh

class Box<T> {
  public function __construct(public T $data) {}
};

function f(): void {
  $a = 42;
  $v = Vector{};
  $w = "";
  $f = (mixed $x) ==> {
    invariant($x is Box<_>, "");
    return new Box($x);
  };
  $b1 = $f(new Box(42));
  $b2 = $f(new Box("oops"));
  // we would not want b1 and b2 to have
  // the same type
}
