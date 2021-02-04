<?hh

class C {
  public function __construct(
    public mixed $f,
  ) {}
}

function f(): void {
  new C(($x, string $y) ==> $y + 1);
}
