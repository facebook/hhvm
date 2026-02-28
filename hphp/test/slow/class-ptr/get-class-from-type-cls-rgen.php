<?hh

class C {
  public static function f(): void { echo "called f\n"; }
}
class G<reify T> {
  public function call() {
    $c = HH\ReifiedGenerics\get_class_from_type<T>();
    $c::f();
  }
}
class G2<reify T2> {
  function call(): void {
    $g = new G<T2>();
    $g->call();
  }
}
abstract class DAbs {
  abstract const type T;
  public static function meth(): void {
    $g = new G<this::T>();
    $g->call();
  }
}
class D extends DAbs {
  const type T = C;
}
<<__EntryPoint>>
function main(): void {
  $g = new G<C>();
  $g->call();
  D::meth();
  $g2 = new G2<C>();
  $g2->call();

  $ge = new G<E>(); // unbound name
  $ge->call(); // not reached
}
