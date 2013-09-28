<?php
namespace A;
use D;
interface I {
  public function foo(D $param);
}

namespace B;
use D;
use A\I;
class E implements I {
  public function foo(D $param) {}
}
echo "ok";
