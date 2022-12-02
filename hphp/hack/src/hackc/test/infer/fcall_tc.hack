// Typecheck Helpers

function f(mixed... $_): void { }

class B {
  public static function bar(): void { }
}

class C {
  public function b(int $a, int $b, int $c): void { }
  public static function f(mixed... $_): void { }
}
