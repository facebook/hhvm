<?hh

class A {}

class B {
  public function foo(): void {
    echo 'foo';
  }
}
class C<T as B> {
  public ?T $item;
}
function TestIt(): void {
  $x = new C();
  $x->item = new A();
}
