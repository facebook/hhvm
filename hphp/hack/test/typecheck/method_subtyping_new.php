<?hh

class Foo {
  public function qq(int $x): num {
    return 5;
  }
}

interface I {
  public function qq(int $x): num;
}

class Bar extends Foo implements I {
  public function qq(num $x): int {
    return 4;
  }

}

abstract class AbstractFoo {
  abstract public function qq(int $x): num;

}

class AbstractBar extends AbstractFoo {
  public function qq(num $x): int {
    return 5;
  }

}
