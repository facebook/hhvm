<?hh

class C {}
class D {
  function __construct() {
    print "In D::__construct()\n";
  }
}
class F {
  function __construct() {
    print "In F::__construct()\n";
  }
}
class G extends C {}
class H extends D {}
class J extends F {}
class K extends H {
  function __construct() {
    print "In K::__construct()\n";
  }
}
class M extends J {
  function M() :mixed{
    print "In M::M()\n";
  }
  function __construct() {
    print "In M::__construct()\n";
  }
}
<<__EntryPoint>>
function main_entry(): void {

  print "Test begin\n";

  $c = new C;
  $c = new D;
  $c = new F;
  $c = new G;
  $c = new H;
  $c = new J;
  $c = new K;
  $c = new M;

  $X = "C"; $c = new $X;
  $X = "D"; $c = new $X;
  $X = "F"; $c = new $X;
  $X = "G"; $c = new $X;
  $X = "H"; $c = new $X;
  $X = "J"; $c = new $X;
  $X = "K"; $c = new $X;
  $X = "M"; $c = new $X;

  print "Test end\n";
}
