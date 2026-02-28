<?hh

class A {
  protected static function invariant_violation(): noreturn {
    throw new Exception();
  }
}

class B extends A {
  public function f(int $x): void {
    switch ($x) {
      case 0:
        echo "zero";
        parent::invariant_violation();
      default:
        echo "more";
    }
  }
}
