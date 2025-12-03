<?hh

class Box<T> {
  public function __construct(public T $item) {}
}

function test<T>(T $x): Box<T> {
  return new Box($x);
}

function test2(): void {
  test(() ==> {}());
}
