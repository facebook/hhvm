<?hh

// These examples should not cause the missing_override_attribute lint error to
// be emitted.

interface I {
  public function i(): int;
  public static function si(): int;
}

interface J {
  public function j(): int;
  public static function sj(): int;
}

class B implements J {
  public function __construct() {}
}

trait T {
  private function t(): int { return 0; }
  private static function st(): int { return 0; }
  public function foo(): int { return 42; } // avoids triggering Lint[5627]
}

class C extends B implements I {
  use T;

  // OK: Constructors do not require the annotation
  public function __construct() {}

  // OK: Implementing an interface does not require the annotation
  public function i(): int { return 0; }
  public static function si(): int { return 0; }

  // OK: Implementing an interface specified by a parent does not require the
  // annotation
  public function j(): int { return 0; }
  public static function sj(): int { return 0; }
}

class D {
  private function f(): void {}
}

class E extends D {
  // OK: Any method matching to private method of base class does not require the annotation
  public function f(): void {}
}
