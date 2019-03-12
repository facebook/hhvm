<?php
namespace NS;

class B {}
class A extends B {
  public function b() {
    \var_dump(self::class);
    \var_dump(static::class);
    \var_dump(parent::class);
  }
}

function c($c = A::class) {
  \var_dump($c);
}

trait C {
  public function c() {
    \var_dump(self::class);
    \var_dump(static::class);
    \var_dump(parent::class);
  }
}
class D extends B {
  use C;
}

interface E {}

<<__EntryPoint>>
function main() {
  \var_dump(A::class);
  A::b();
  c();
  \var_dump(Vector::class);
  D::c();
  \var_dump(C::class);
  \var_dump(E::class);
}
