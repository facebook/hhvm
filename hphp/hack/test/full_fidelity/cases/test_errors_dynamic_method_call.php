<?hh

class C {
  public function f<T>() {}
}
class D {
  public static function f<T>() {}
}

$x = 'f';
$c = new C();

$c->f<int>();  // ok
$c->$x<int>(); // parse error

$c?->f<int>(); // ok
$c?->$x<int>(); // parse errors

D::f<int>();   // ok
D::$x<int>();  // parse error
