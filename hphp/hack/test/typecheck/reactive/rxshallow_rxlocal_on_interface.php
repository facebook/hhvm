<?hh // strict
interface Rx {

  public function f1(): int;

  public function f2(): int;
}

class A {

  public function f1(): int {
    return 1;
  }


  public function f2(): int {
    return 2;
  }
}

class B extends A {
  // OK
  <<__Override>>
  public function f1(): int {
    return 1;
  }
  // OK
  <<__Override>>
  public function f2(): int {
    return 1;
  }
}
