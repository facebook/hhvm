<?hh // strict

abstract class A {
  public abstract function nonrx(): int;
  <<__RxLocal>>
  public abstract function localrx(): int;
  <<__RxShallow>>
  public abstract function shallowrx(): int;
  <<__Rx>>
  public abstract function rx(): int;
}

abstract class A0 extends A {
  // OK to override non-rx with rxlocal
  <<__Override, __RxLocal>>
  public function nonrx(): int {
    return 1;
  }
  // OK to override rx-local with rx-local
  <<__Override, __RxLocal>>
  public function localrx(): int {
    return 1;
  }
}

abstract class A1 extends A {
  // OK to override rx-local with rx-shallow
  <<__Override, __RxShallow>>
  public function localrx(): int {
    return 1;
  }
  // OK to override rx-shallow with rx-shallow
  <<__Override, __RxShallow>>
  public function shallowrx(): int {
    return 1;
  }
}

class B extends A {
  // OK to override non-rx with rx
  <<__Override, __Rx>>
  public function nonrx(): int {
    return 1;
  }
  // OK to override local-rx with rx
  <<__Override, __Rx>>
  public function localrx(): int {
    return 1;
  }
  // OK to override shallow-rx with rx
  <<__Override, __Rx>>
  public function shallowrx(): int {
    return 1;
  }
  // OK to override rx with rx
  <<__Override, __Rx>>
  public function rx(): int {
    return 1;
  }
}
