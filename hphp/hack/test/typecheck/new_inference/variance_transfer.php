<?hh // strict

class Cov<+T> {
  public function get(): ?T {
    return null;
  }
}

class A<+T1, -T2 as Cov<T1>> {
  private Vector<?T1> $x;
  public function __construct() {
    $this->x = new Vector(null);
  }
  public function put(T2 $x): void {
    $this->x[] = $x->get();
  }
  public function get(): ?T1 {
    return $this->x[0];
  }
}

function test(): void {
  $x = new A();
  take_mixed($x->get());
  take_arraykey_opt($x->get());
  $x->put(new Cov<int>());
  $x->put(new Cov<string>());
}

function take_mixed(mixed $x): void {}
function take_arraykey_opt(?arraykey $x): void {}
