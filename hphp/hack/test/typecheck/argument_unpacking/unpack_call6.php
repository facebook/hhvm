<?hh // partial

class C1 {
  public function __construct(
    private int $foo,
    private ...$args
  ) {}
}

function test() {
  $args = 'string';
  new C1(...$args); // should be error
}
