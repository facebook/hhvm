<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class C {
  abstract const type T;
}

class Box<T> {
  public function __construct(public T $data) {}
}

function mk<T>(): T
  where T = Box<?C with {type T = T}>
{ return new Box(null); }

function occurs_check(): void {
  mk();
}
