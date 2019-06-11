<?hh // strict
interface IFoo {
  const type T = int;

  public function get(): this::T;
}

class Foo implements IFoo {
  public function __construct(private this::T $x) {}

  public function get(): this::T {
    return $this->x;
  }
}

function error(IFoo $foo): void {
  if ($foo is Foo) {
    $foo->get();
  }
  hh_show($foo);
  $foo->get();
}
