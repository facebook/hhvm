<?hh

class A {}

class B {
  public function returns_readonly(): readonly A {
    invariant_violation('');
  }
}

function test(): void {
  $x = meth_caller(B::class, 'returns_readonly');
}
