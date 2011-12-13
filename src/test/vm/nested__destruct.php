<?

print "Test begin\n";

class C {
  public $c;
  function __construct($c=null, $d=null) {
    $this->c = $c;
    $this->d = $d;
  }
  function __destruct() {
    print "In C::__destruct()\n";
  }
}

$c = new C(new C(), new C());
$c = null;

print "Test end\n";
