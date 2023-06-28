<?hh

class C {
  function __construct() {
    print "In C::__construct()\n";
  }
  static function sMeth() :mixed{
    print "In C::sMeth()\n";
  }
}

class F extends C {}

class I extends F {
  function __construct() {
    print "In I::__construct()\n";
  }
}

function anon($o) :mixed{
  print "In anon()\n";
//  $o->fPrivate();
//  $o->fProtected();
  $o->fPublic();
}
class W {
  static public function fW($o) :mixed{
    print "In W::fW()\n";
//    $o->fPrivate();
//    $o->fProtected();
    $o->fPublic();
  }
}
class X {
  private function fPrivateX() :mixed{
    print "  In X::fPrivateX()\n";
  }
  private function fPrivate() :mixed{
    print "  In X::fPrivate()\n";
  }
  protected function fProtected() :mixed{
    print "  In X::fProtected()\n";
  }
  public function fPublic() :mixed{
    print "  In X::fPublic()\n";
  }
  public function fX() :mixed{
    print "In X::fX()\n";
    $this->fPrivateX();
    $this->fPrivate();
    $this->fProtected();
    $this->fPublic();
  }
}
class Y extends X {
  private function fPrivateY() :mixed{
    print "  In Y::fPrivateY()\n";
  }
  private function fPrivate() :mixed{
    print "  In Y::fPrivate()\n";
  }
  protected function fProtected() :mixed{
    print "  In Y::fProtected()\n";
  }
  public function fPublic() :mixed{
    print "  In Y::fPublic()\n";
  }
  public function fY() :mixed{
    print "In Y::fY()\n";
    $this->fPrivateY();
    $this->fPrivate();
    $this->fProtected();
    $this->fPublic();
  }
}
class Z extends Y {
  private function fPrivateZ() :mixed{
    print "  In Z::fPrivateZ()\n";
  }
  private function fPrivate() :mixed{
    print "  In Z::fPrivate()\n";
  }
  protected function fProtected() :mixed{
    print "  In Z::fProtected()\n";
  }
  public function fPublic() :mixed{
    print "  In Z::fPublic()\n";
  }
  public function fZ() :mixed{
    print "In Z::fZ()\n";
    $this->fPrivateZ();
    $this->fPrivate();
    $this->fProtected();
    $this->fPublic();
  }
}
<<__EntryPoint>>
function entrypoint_FPushObjMethod(): void {

  print "Test begin\n";

  $c = new C;
    $c->__construct();
    $c::sMeth();
  $c = new F;
    $c->__construct();
  $c = new I;
    $c->__construct();

  $c = new C;
    $m = "__construct"; $c->$m();
    $m = "sMeth"; $c::$m();
  $c = new F;
    $m = "__construct"; $c->$m();
  $c = new I;
    $m = "__construct"; $c->$m();

  print "=== X ===\n";
  $x = new X();
  anon($x);
  W::fW($x);
  $x->fX();
  //$x->fY();
  //$x->fZ();
  print "=== Y ===\n";
  $y = new Y();
  anon($y);
  W::fW($y);
  $y->fX();
  $y->fY();
  //$y->fZ();
  print "=== Z ===\n";
  $z = new Z();
  anon($z);
  W::fW($z);
  $z->fX();
  $z->fY();
  $z->fZ();

  $not_a_string = 123;
  $foo = C::$not_a_string();
}
