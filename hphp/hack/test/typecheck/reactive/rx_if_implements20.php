<?hh // strict
interface Rx1 {}

abstract class A {
  public abstract function nonrx(): int;

  public abstract function localrx(): int;

  public abstract function condlocalrx(): int;

  public abstract function shallowrx(): int;

  public abstract function condshallowrx(): int;

  public abstract function rx(): int;

  public abstract function condrx(): int;
}

abstract class A0 extends A {
  // ok to override local with local
  <<__Override>>
  public function localrx(): int {
    return 1;
  }
  // ok to override cond local with local (if condition is not met - method
  // in base class is non-reactive )
  <<__Override>>
  public function condlocalrx(): int {
    return 1;
  }
}

abstract class A1 extends A {
  // ok to override local with shallow
  <<__Override>>
  public function localrx(): int {
    return 1;
  }
  // ok to override cond local with shallow (if condition is not met - method
  // in base class is non-reactive )
  <<__Override>>
  public function condlocalrx(): int {
    return 1;
  }
  // ok to override shallow with shallow
  <<__Override>>
  public function shallowrx(): int {
    return 1;
  }
  // ok to override cond shallow with shallow (if condition is not met - method
  // in base class is non-reactive )
  <<__Override>>
  public function condshallowrx(): int {
    return 1;
  }
}

abstract class A2 extends A {
  // ok to override local with rx
  <<__Override>>
  public function localrx(): int {
    return 1;
  }
  // ok to override cond local with rx (if condition is not met - method
  // in base class is non-reactive )
  <<__Override>>
  public function condlocalrx(): int {
    return 1;
  }
  // ok to override shallow with rx
  <<__Override>>
  public function shallowrx(): int {
    return 1;
  }
  // ok to override cond shallow with rx (if condition is not met - method
  // in base class is non-reactive )
  <<__Override>>
  public function condshallowrx(): int {
    return 1;
  }
  // ok to override rx with rx
  <<__Override>>
  public function rx(): int {
    return 1;
  }
  // ok to override cond rx with rx (if condition is not met - method
  // in base class is non-reactive )
  <<__Override>>
  public function condrx(): int {
    return 1;
  }
}
