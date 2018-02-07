<?hh

class C1 {
  public function __construct(private string $foo) {}
}

function test(): void {
  $args = array();
  // arity error
  new C1('string', ...$args);
}
