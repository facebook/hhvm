<?hh

class C {
  public function __construct(private int $x) {}
  public function get(): int { return $this->x; }
}

function f(): void {
  $c1 = new C(1);
  (($c, $d, $e) ==> $c->get() + $d)($c1, 2, 1 + 2);
  new C(1) |> (($c, $d) ==> $c->get() + $d)($$, 2);
}
