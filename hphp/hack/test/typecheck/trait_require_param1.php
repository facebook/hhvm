<?hh // partial

abstract class Foo<T> {
  /* HH_FIXME[4336] */
  protected function bar(): T {
  }
}

trait Tbaz {
  require extends Foo<int>;

  protected function f(): void {
    $int = $this->bar();
    f_takes_int($int);
  }
}

function f_takes_int(int $f): void {}
