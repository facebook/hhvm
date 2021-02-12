<?hh // strict
interface Rx {

  public function f(int $a): void;
}


class A<T> {

  public function f(T $a): void {
  }
}

class B extends A<int> implements Rx {
}
