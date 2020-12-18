<?hh

class Box<T> {
  public T $p;
  public function __construct(T $x) {
    $this->p = $x;
  }
  public function get() : T {
    return $this->p;
  }
  public function set(T $x) : void {
    $this->p = $x;
  }
}

function f(Box<int> $x) : void {
  $y = $x as dynamic;
}
