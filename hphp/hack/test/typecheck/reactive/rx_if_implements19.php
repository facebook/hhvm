<?hh // strict
abstract class A {
  public abstract function nonrx(): int;

  public abstract function localrx(): int;

  public abstract function shallowrx(): int;

  public abstract function rx(): int;
}

abstract class A0 extends A {
  // OK to override non-rx with rxlocal
  <<__Override>>
  public function nonrx(): int {
    return 1;
  }
  // OK to override rx-local with rx-local
  <<__Override>>
  public function localrx(): int {
    return 1;
  }
}

abstract class A1 extends A {
  // OK to override rx-local with rx-shallow
  <<__Override>>
  public function localrx(): int {
    return 1;
  }
  // OK to override rx-shallow with rx-shallow
  <<__Override>>
  public function shallowrx(): int {
    return 1;
  }
}

class B extends A {
  // OK to override non-rx with rx
  <<__Override>>
  public function nonrx(): int {
    return 1;
  }
  // OK to override local-rx with rx
  <<__Override>>
  public function localrx(): int {
    return 1;
  }
  // OK to override shallow-rx with rx
  <<__Override>>
  public function shallowrx(): int {
    return 1;
  }
  // OK to override rx with rx
  <<__Override>>
  public function rx(): int {
    return 1;
  }
}
