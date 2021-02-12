<?hh // strict
interface Rx {

  public function f(int $a): void;
}

trait TRx implements Rx {
}

class A<T> {

  public function f(T $a): void {
  }
}

class B extends A<int> {
  use TRx;
}


function f(B $b): void {
  $b->f(1);
}
