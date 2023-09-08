<?hh

class C {
  public static function bar<reify T>(): void {}
}

function foo(): ?classname<C> { return C::class; }

function main(): void {
  $c = foo();
  if ($c is nonnull) {
    $c::bar<int>();
  }
}
