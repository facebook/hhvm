<?hh // strict

interface Rx1 {}

abstract class A {
  public abstract function nonrx(): int;
  <<__RxLocal>>
  public abstract function localrx(): int;
  <<__RxLocal, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function condlocalrx(): int;
  <<__RxShallow>>
  public abstract function shallowrx(): int;
  <<__RxShallow, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function condshallowrx(): int;
  <<__Rx>>
  public abstract function rx(): int;
  <<__Rx, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function condrx(): int;
}

abstract class A0 extends A {
  // ok to override local with local
  <<__Override, __RxLocal>>
  public function localrx(): int {
    return 1;
  }
  // ok to override cond local with local (if condition is not met - method
  // in base class is non-reactive )
  <<__Override, __RxLocal>>
  public function condlocalrx(): int {
    return 1;
  }
}

abstract class A1 extends A {
  // ok to override local with shallow
  <<__Override, __RxShallow>>
  public function localrx(): int {
    return 1;
  }
  // ok to override cond local with shallow (if condition is not met - method
  // in base class is non-reactive )
  <<__Override, __RxShallow>>
  public function condlocalrx(): int {
    return 1;
  }
  // ok to override shallow with shallow
  <<__Override, __RxShallow>>
  public function shallowrx(): int {
    return 1;
  }
  // ok to override cond shallow with shallow (if condition is not met - method
  // in base class is non-reactive )
  <<__Override, __RxShallow>>
  public function condshallowrx(): int {
    return 1;
  }
}

abstract class A2 extends A {
  // ok to override local with rx
  <<__Override, __Rx>>
  public function localrx(): int {
    return 1;
  }
  // ok to override cond local with rx (if condition is not met - method
  // in base class is non-reactive )
  <<__Override, __Rx>>
  public function condlocalrx(): int {
    return 1;
  }
  // ok to override shallow with rx
  <<__Override, __Rx>>
  public function shallowrx(): int {
    return 1;
  }
  // ok to override cond shallow with rx (if condition is not met - method
  // in base class is non-reactive )
  <<__Override, __Rx>>
  public function condshallowrx(): int {
    return 1;
  }
  // ok to override rx with rx
  <<__Override, __Rx>>
  public function rx(): int {
    return 1;
  }
  // ok to override cond rx with rx (if condition is not met - method
  // in base class is non-reactive )
  <<__Override, __Rx>>
  public function condrx(): int {
    return 1;
  }
}
