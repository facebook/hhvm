<?hh

class C {
  public static function bar<reify T>(): void {}
}

function foo(): ?classname<C> { return nameof C; }

function main(): void {
  $c = foo();
  if ($c is nonnull) {
    $c::bar<int>();
  }
}
