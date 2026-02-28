<?hh

interface I {
  public function f($a):mixed;
}

class B {
}

class C extends B implements I {
  const k = 42;

  public static $s = "s";

  public $p0;
  public $p1 = null;
  public $p = 13;
  protected $q = "q...";
  private $r = vec[1, 2, 3];

  function __construct() {
    print "In C::__construct()\n";
  }

  function f($a) :mixed{
    // None-bare $this.
    print "In C::f(), p : ".$this->p."\n";
    print "In C::f(), q : ".$this->q."\n";
    print "In C::f(), r : ".$this->r."\n";
    $this->g(
    // Bare $this.
             $this);
    $x = $this;
    $this;
    return $this;
  }

  function g($x) :mixed{
    // None-bare $this.
    print "In C::g(), p : ".$this->p."\n";
    print "In C::g(), q : ".$this->q."\n";
    print "In C::g(), r : ".$this->r."\n";
  }

  static function sf() :mixed{
    print "In C::sf()\n";
  }
}

function main() :mixed{
  print "C::k : ".C::k."\n";
  $X = "C";
  print "$X::k : ".$X::k."\n";

// XXX Exit here to avoid unimplemented HHBC instructions.
  exit(0);

  C::sf();

// XXX Need SProp replacement.
//print "C::\$s : ".C::$s."\n";
//print "\$X::\$s : ".$X::$s."\n";

  $c = new C();
  print "\$c->p : ".$c->p."\n";
  $c->f(43);
  var_dump($c);

  print "Test end\n";
}
<<__EntryPoint>>
function main_entry(): void {

  print "Test begin\n";
  main();
}
