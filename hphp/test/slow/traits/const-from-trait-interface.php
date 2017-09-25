<?hh

interface I {
  const v = 42;
}
trait T implements I {}
class C {
  use T;
  const v = 42;
}
class D extends C {}
$c = new D();
var_dump($c instanceof I);
