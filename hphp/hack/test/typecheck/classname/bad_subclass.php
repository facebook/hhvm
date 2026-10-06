<?hh

interface I {}
class C_isI implements I {}
class C_notI {}

abstract class Super {
  public abstract function nameOfISubclass(): classname<I>;
}

class GoodSub extends Super {
  public function nameOfISubclass(): classname<I> {
    return nameof C_isI;
  }
}

class BadSub extends Super {
  public function nameOfISubclass(): classname<C_notI> {
    return nameof C_notI;
  }
}
