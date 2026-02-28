<?hh

enum class E : int {
  int AAAA = 42;
  int BBBB = 0;
}

class C {
  const type T = this;

  public function f(): this {
    return $this;
  }

  public function g(HH\EnumClass\Label<E, int> $x): void {}
}

function main(): void {
  $c = new C();
  $c->f()->g(#AUTO332 // g has type dynamic | supportdyn<Tfun ...>
