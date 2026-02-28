<?hh

class C<reify T> {
  public function f(): void {
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
  }
}

final class D extends C<int> {}

<<__EntryPoint>>
function main(): void {
  $x = D::class;
  $y = new $x();
  $y->f();
}
