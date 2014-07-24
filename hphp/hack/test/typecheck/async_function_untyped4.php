<?hh

interface I1 {
  public function f(): int;
}

class C1 implements I1 {
  public async function f() { return 10; }
}
