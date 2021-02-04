<?hh

class C {
  public function __construct(
    public mixed $f,
  ) {}
}

function f(): void {
  new C(($x) : int ==> $x);
}
