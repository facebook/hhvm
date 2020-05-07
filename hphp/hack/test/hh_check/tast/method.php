<?hh

class Box<T> {
  public function __construct(private T $x) {}
  public function get(): T {
    return $this->x;
  }
}

function mk_box(): void {
  (new Box(1))->get();
}
