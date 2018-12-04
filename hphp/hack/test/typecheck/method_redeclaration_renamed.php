<?hh // strict

trait T {
  public function f(): int {
    return 4;
  }
}

trait T2 {
  public function f(): string {
    return "4";
  }
}

class C {
  use T, T2;

  public function g(): int = T::f;
}

function f(): void {
  $c = new C();
  hh_show($c->f());
  hh_show($c->g());
}
