<?hh

trait MyTrait {
  public function foo(): void {}
}

function test(): void {
  meth_caller('MyTrait', 'foo');
}
