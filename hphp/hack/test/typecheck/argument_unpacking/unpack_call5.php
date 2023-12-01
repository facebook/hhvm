<?hh

class C1 {
  public function __construct(
    private int $foo,
    private mixed ...$args
  ) {}
}

function test(): void {
  $args = vec[1, 2, 3];
  // positional args should be typechecked
  new C1('string', ...$args);
}
