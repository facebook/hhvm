<?hh

class A {
  public function __toString(): string {
    return "A";
  }
}

abstract class B {
  public abstract function __toString(): string;
}

interface SomeOtherC {
  public function __toString(): string;
}

trait TA {
  public function __toString(): string {
    return "TA";
  }
}
