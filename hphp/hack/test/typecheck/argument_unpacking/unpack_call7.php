<?hh // partial

class C1 {
  public function __construct(private string $foo) {}
}

function test(): void {
  $args = varray[];
  // arity error
  new C1('string', ...$args);
}
