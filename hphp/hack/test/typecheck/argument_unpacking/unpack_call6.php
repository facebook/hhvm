<?hh

class C1 {
  public function __construct(
    private int $foo,
    private mixed ...$args
  ) {}
}

function test(): void {
  $args = 'string';
  new C1(...$args); // should be error
}
