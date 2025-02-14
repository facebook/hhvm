<?hh

final class Foo {}

interface IMyGenericTaker<T> {
  public function takes(T $t): void;
}

abstract class FooTaker implements IMyGenericTaker<Foo> {
}

abstract class FooTakerChild extends FooTaker {
}

class MyClass<T> {

  public function __construct(private IMyGenericTaker<T> $taker, private T $t) {
  }

  public async function split(): Awaitable<void> {
    $taker = $this->taker;
    if ($taker is FooTaker || $taker is FooTakerChild) {
      // if $taker is FooTaker, then T = Foo, so $this->t : Foo
      // `$taker is FooTakerChild` is redundant, but it should not affect this
      // refinement of T
      $taker->takes($this->t);
    }
  }
}
