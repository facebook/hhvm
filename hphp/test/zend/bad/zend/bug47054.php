<?php

class C {
  final static function s() {
    print "Called class: " . get_called_class() . "\n";
  }
}
class D extends C {
  public function m() {
    $this->s();
  }
}

$d = new D();
$d->m();

C::s();

$c = new C();
$c->s();

get_called_class();

D::m();

?>