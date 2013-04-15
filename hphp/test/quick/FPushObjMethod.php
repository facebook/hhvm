<?php

print "Test begin\n";

class C {
  function __construct() {
    print "In C::__construct()\n";
  }
  static function sMeth() {
    print "In C::sMeth(): \$this:" . (isset($this) ? $this : null) . ":\n";
  }
}
class D {
  function D() {
    print "In D::D()\n";
  }
}
class E {
  function E() {
    print "In E::E()\n";
  }
  function __construct() {
    print "In E::__construct()\n";
  }
}

class F extends C {}
class G extends D {}
class H extends E {}

class I extends F {
  function __construct() {
    print "In I::__construct()\n";
  }
}
class J extends G {
  function J() {
    print "In J::J()\n";
  }
}
class K extends H {
  function K() {
    print "In K::K()\n";
  }
  function __construct() {
    print "In K::__construct()\n";
  }
}

$c = new C;
  $c->__construct();
  $c->sMeth();
$c = new D;
  $c->D();
$c = new E;
  $c->E();
  $c->__construct();
$c = new F;
  $c->__construct();
$c = new G;
  $c->D();
$c = new H;
  $c->E();
  $c->__construct();
$c = new I;
  $c->__construct();
$c = new J;
  $c->D();
  $c->J();
$c = new K;
  $c->E();
  $c->K();
  $c->__construct();

$c = new C;
  $m = "__construct"; $c->$m();
  $m = "sMeth"; $c->$m();
$c = new D;
  $m = "D"; $c->$m();
$c = new E;
  $m = "E"; $c->$m();
  $m = "__construct"; $c->$m();
$c = new F;
  $m = "__construct"; $c->$m();
$c = new G;
  $m = "D"; $c->$m();
$c = new H;
  $m = "E"; $c->$m();
  $m = "__construct"; $c->$m();
$c = new I;
  $m = "__construct"; $c->$m();
$c = new J;
  $m = "D"; $c->$m();
  $m = "J"; $c->$m();
$c = new K;
  $m = "E"; $c->$m();
  $m = "K"; $c->$m();
  $m = "__construct"; $c->$m();

function anon($o) {
  print "In anon()\n";
#  $o->fPrivate();
#  $o->fProtected();
  $o->fPublic();
}
class W {
  static public function fW($o) {
    print "In W::fW()\n";
#    $o->fPrivate();
#    $o->fProtected();
    $o->fPublic();
  }
}
class X {
  private function fPrivateX() {
    print "  In X::fPrivateX()\n";
  }
  private function fPrivate() {
    print "  In X::fPrivate()\n";
  }
  protected function fProtected() {
    print "  In X::fProtected()\n";
  }
  public function fPublic() {
    print "  In X::fPublic()\n";
  }
  public function fX() {
    print "In X::fX()\n";
    $this->fPrivateX();
    $this->fPrivate();
    $this->fProtected();
    $this->fPublic();
  }
}
class Y extends X {
  private function fPrivateY() {
    print "  In Y::fPrivateY()\n";
  }
  private function fPrivate() {
    print "  In Y::fPrivate()\n";
  }
  protected function fProtected() {
    print "  In Y::fProtected()\n";
  }
  public function fPublic() {
    print "  In Y::fPublic()\n";
  }
  public function fY() {
    print "In Y::fY()\n";
    $this->fPrivateY();
    $this->fPrivate();
    $this->fProtected();
    $this->fPublic();
  }
}
class Z extends Y {
  private function fPrivateZ() {
    print "  In Z::fPrivateZ()\n";
  }
  private function fPrivate() {
    print "  In Z::fPrivate()\n";
  }
  protected function fProtected() {
    print "  In Z::fProtected()\n";
  }
  public function fPublic() {
    print "  In Z::fPublic()\n";
  }
  public function fZ() {
    print "In Z::fZ()\n";
    $this->fPrivateZ();
    $this->fPrivate();
    $this->fProtected();
    $this->fPublic();
  }
}

print "=== X ===\n";
$x = new X();
anon($x);
W::fW($x);
$x->fX();
#$x->fY();
#$x->fZ();
print "=== Y ===\n";
$y = new Y();
anon($y);
W::fW($y);
$y->fX();
$y->fY();
#$y->fZ();
print "=== Z ===\n";
$z = new Z();
anon($z);
W::fW($z);
$z->fX();
$z->fY();
$z->fZ();

$not_a_string = 123;
$foo = C::$not_a_string();
