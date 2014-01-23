<?php
namespace NS;
function main() {
  class B {}
  class A extends B {
    public function b() {
      var_dump(self::class);
      var_dump(static::class);
      var_dump(parent::class);
    }
  }
  var_dump(A::class);
  A::b();

  function c($c = A::class) {
    var_dump($c);
  }
  c();

  var_dump(Vector::class);

  trait C {
    public function c() {
      var_dump(self::class);
      var_dump(static::class);
      var_dump(parent::class);
    }
  }
  class D extends B {
    use C;
  }
  D::c();
  var_dump(C::class);

  interface E {}
  var_dump(E::class);
}
main();
