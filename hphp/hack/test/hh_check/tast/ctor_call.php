<?hh

class Box<T> {
  public function __construct(private T $x) {}
}

function mk_box(): void {
  new Box(1);
  new Box("");
}
