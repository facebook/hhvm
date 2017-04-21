<?hh

function f(
  $foo,
  string $bar = 'aaaa',
) {
}

function g(
  ...$varargs
) {
}

class C {

  public function __construct(
    public string $x = g(),
  ) {}

  public function h(int $method_param) {}
}
