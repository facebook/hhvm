<?hh

class Box<T> { public function __construct(public T $x) {} }

class BoxA<T as arraykey> extends Box<T> {}

function fn(arraykey $a): void {}

abstract class A {
  abstract const type T;

  abstract public function get(): vec<Box<this::T>>;

  public function test(): void {
    foreach ($this->get() as $x) {
      ((Box<this::T> $x) ==> {
          invariant($x is BoxA<_>, "");
          // we now have a local constraint that this::T <: arraykey
          // make sure the escaping logic does not pick up an error
          // here by looking at the tpenv and spotting this::T there
          fn($x->x);
      })($x);
    }
  }
}
