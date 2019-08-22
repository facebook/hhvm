<?hh //strict

class A {
  /* HH_FIXME[4336] */
  protected static function invariant_violation(): noreturn {
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
