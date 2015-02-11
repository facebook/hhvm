<?hh

class X {
  public function __construct(private int $i) {}
  public function __destruct(): void { echo "dtor: $this->i\n"; }
  public function __toString(): string { return "x"; }
}

function err(): void {}

function foo(): void {
  set_error_handler('err');
  hash(new X(1), new X(2));
}
foo();
