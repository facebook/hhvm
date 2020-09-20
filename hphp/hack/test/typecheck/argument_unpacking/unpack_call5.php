<?hh // partial

class C1 {
  public function __construct(
    private int $foo,
    private ...$args
  ) {}
}

function test(): void {
  $args = varray[1, 2, 3];
  // positional args should be typechecked
  new C1('string', ...$args);
}
