<?hh

class C<reify T> {}

class D {
  const type T = int;
  public function g<reify T>(): void {}
  public function f(
    C<this::T> $t /* ok */
  ): C<this::T> /* ok */ {
    $this->g<this::T>(); /* ok */
    return new C<this::T>(); /* ok */
  }
}
