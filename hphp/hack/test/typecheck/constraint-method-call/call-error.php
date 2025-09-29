<?hh

class Inv<T> {
  public function __construct(private T $x) {}
}

class C {
  public function f(int $t, string $u): void {}

  public function g(): void {
    $this->f(1, 2); // The primary position should be the expression 2
  }

  public function g2(): void {
    $f = $x ==> $x->f(1, 2);
    $f($this); // The primary position should be the $this
  }

  public function h(): void {
    $this->missing(); // The primary position should be the missing
  }

  public function h2(): void {
    $f = $x ==> $x->missing();
    $f($this); // The primary position should be the $this
  }

  public function i((function (int, string): void) $f): void {
  }

  public function i2((function (int, int): void) $f): void {
    $this->i($f); // The primary position should be $f
  }

  public function j<T as arraykey>(T $x) : void {
  }

  public function k(): void {
    $this->j(true); // The primary position should be the true expression
  }

  public function l<T as string>(T $x): void {
  }

  public function l2(): void {
    $this->l(1); // The primary position should be the 1
  }

  public function l3<T as vec<string>>(T $x): void {
  }

  public function l4(): void {
    $this->l3(vec[1]); // The primary position should be $x
  }

  public function m<T as Vector<string>>(T $x): void {}

  public function m2(): void {
    $this->m(Vector{1}); // The primary position should be the Vector, but is the whole expression.
  }
}
