<?

class F {
  public $foo;
}

function t($o, $memb) {
  var_dump($o->$memb);
  unset($o->$memb);
  var_dump($o->$memb);
}

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

function u() {
  echo "------------------------\n";
  $obj = new F;
  $obj->foo = $x;
  foreach ($obj as $k => $_) {
    echo $k . "\n";
  }
  echo "------------------------\n";
  $obj = new F;
  $obj->foo = $y++;
  foreach ($obj as $k => $_) {
    echo $k . "\n";
  }
}

function main() {
  print "Test begin\n";

  $f = new F();
  $f->foo = 12;
  $f->bart = "snoot";
  var_dump($f);

  t($f, 'foo');
  t($f, 'bart');
  var_dump($f);

  $c = new C();
  var_dump($c);
  $c->cF();
  var_dump($c);

  $e = error_reporting(0);
  u();

  error_reporting($e);
  print "Test end\n";
}

main();

function getprop($o) {
  return $o->declprop;
}
function setprop($o, $v) {
  $o->declprop = $v;
}
class c2 {
  public $declprop = 'blah';
}

function main2() {
  $o = new c2();
  setprop($o, 'set1');
  var_dump(getprop($o));
  unset($o->declprop);
  var_dump(getprop($o));
  setprop($o, 'set2');
  var_dump(getprop($o));
}

main2();
