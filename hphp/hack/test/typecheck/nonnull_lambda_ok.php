<?hh

class C {
  public function __construct(
    public nonnull $f,
  ) {}
}

function f(): void {
  new C($x ==> $x);
}
