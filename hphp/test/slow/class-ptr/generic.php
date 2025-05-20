<?hh

class X {}
class C<T> {
  public function __construct() { echo "new\n"; }
}

function f (
  class<C<X>> $c,
): void {
  new C();
}

<<__EntryPoint>>
function main(): void {
  f(C::class);
}
