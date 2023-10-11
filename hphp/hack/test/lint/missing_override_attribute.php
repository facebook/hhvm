<?hh

abstract class A {
  abstract public function a(): int;
  abstract public static function sa(): int;
}

class B extends A {
  public function b(): int { return 0; }
  public static function sb(): int { return 0; }
}

interface I {
  public function i(): num;
  public static function si(): num;
}

trait T {
  public function t(): int { return 0; }
  public static function st(): int { return 0; }
  public function foo(): int { return 42; } // avoids triggering Lint[5627]
}

class C extends B {
  use T;

  // BAD: An "override" of an abstract method requires the annotation
  public function a(): int { return 1; }
  public static function sa(): int { return 1; }

  // BAD: An override of a concrete method requires the annotation
  public function b(): int { return 1; }
  public static function sb(): int { return 1; }

  // BAD: An override of a trait method requires the annotation
  public function t(): int { return 1; }
  public static function st(): int { return 1; }
}

interface J extends I {
  // BAD: In an interface, overriding a method inherited from another interface
  // requires the annotation
  public function i(): int;
  public static function si(): int;

}

class D {
  use T;
  // BAD: An override of a trait method requires the annotation
  private function t(): int { return 1; }
  private static function st(): int { return 1; }
}
