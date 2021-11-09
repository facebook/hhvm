<?hh

class C {
  public function __construct(
    public dynamic $f,
  ) {}
}

function f(): void {
  new C($x ==> $x + 1);
}
