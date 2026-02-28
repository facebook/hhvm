<?hh
class C {
  public function foo(): D {
    return new D();
  }
}
class D {}
function test(?C $c): D {
  return $c?->foo();
}
