<?hh

interface I {
  const int X = 4;
}
abstract class A implements I {
  public static function f(): void {
    var_dump(nameof static);
  }
}
class C extends A {}
final class D extends C {}

<<__EntryPoint>>
function main(): void {
  $ps = class_parents(D::class);
  foreach ($ps as $p) {
    $p::f();
  }
  $is = class_implements(D::class);
  foreach ($is as $i) {
    var_dump($i::X);
  }
}
