<?hh // strict

interface I {}
class C_isI implements I {}
class C_notI {}

abstract class Super {
  public abstract function nameOfISubclass(): classname<I>;
}

class GoodSub extends Super {
  public function nameOfISubclass(): classname<I> {
    return C_isI::class;
  }
}

class BadSub extends Super {
  public function nameOfISubclass(): classname<C_notI> {
    return C_notI::class;
  }
}
