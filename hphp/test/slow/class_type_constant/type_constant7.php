<?hh

interface I {
  abstract const type T;

  public function foo(this::T $t): void;
}

class IMixed implements I {
  const type T = mixed;

  public function foo(mixed $t): void {}
}

class IFunction implements I {
  const type T = (function(): void);

  public function foo((function(): void) $t): void {}
}

class IString implements I {
  const type T = string;

  public function foo(string $t): void {}
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
