<?hh
interface Rx {}

class A {

  public function f(): int {
    return 1;
  }

  public static function f1(): int {
    return 1;
  }
}

class B extends A implements Rx {

  public function g(): int {
    // OK to call static and instance methods
    return parent::f() + parent::f1();
  }

  public static function g1(): int {
    // OK
    return parent::f1();
  }
}
