<?hh

class C1 {
  public async function f() { return 10; }
}

class C2 extends C1 {
  public function f(): int { return 10; }
}
