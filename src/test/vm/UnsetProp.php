<?

print "Test begin\n";

class F {
  public $foo;
}
$f = new F();
$f->foo = 12;
$f->bart = "snoot";
var_dump($f);

function t($o, $memb) {
  var_dump($o->$memb);
  unset($o->$memb);
  var_dump($o->$memb);
}
t($f, 'foo');
t($f, 'bart');
var_dump($f);

class C {
  public $p = "C::p";
  public $q;
  function __unset($propName) {
    print "In C::__unset(\"$propName\")\n";
  }
  function cF() {
    unset($this->p);
    unset($this->q);
    unset($this->r);
  }
}

$c = new C();
var_dump($c);
$c->cF();
var_dump($c);

print "Test end\n";
