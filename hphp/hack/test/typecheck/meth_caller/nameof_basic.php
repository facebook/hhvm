<?hh

class C {
  public function foo(): void {}
}

function test(): void {
  meth_caller(nameof C, 'foo');
}
