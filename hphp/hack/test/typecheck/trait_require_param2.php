<?hh

abstract class Foo<T> {
  protected function bar(): T {
    // UNSAFE
  }
}

trait Tbaz {
  require extends Foo<float>;

  protected function f(): void {
    $int = $this->bar();
    f_takes_int($int);
  }
}

function f_takes_int(int $f): void {}
