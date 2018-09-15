<?php

function obj_dump($o) {
  var_dump($o);
  print "Properties:\n";
  $a = (array)$o;
  $i = 0;
  foreach ($a as $k => $v) {
    print "  \"";
    for ($i = 0; $i < strlen($k); $i++) {
      $c = $k[$i];
      if ($c != chr(0)) {
        print "$c";
      } else {
        print "\\0";
      }
    }
    print "\" => ".$v."\n";
  }
}
class C {
  const C = "C::C";
  private $np = C::C;
  protected $nq = C::C;
  public $nr = C::C;
  private $p = 1;
  protected $q = 2;
  public $r = 3;
  function __construct() {
    $this->s = 0;
  }
  function fC() {
    $this->np++;
    $this->nq++;
    $this->nr++;
    $this->p++;
    $this->q++;
    $this->r++;
    $this->s++;
  }
}
class D extends C {
  const D = "D::D";
  private $np = D::D;
  protected $nq = D::D;
  public $nr = D::D;
  private $p;
  protected $q;
  public $r;
  function __construct() {
    C::__construct();
    $this->p = 11;
    $this->q = 22;
    $this->r = 33;
  }
  function fD() {
    $this->np++;
    $this->nq++;
    $this->nr++;
    $this->p++;
    $this->q++;
    $this->r++;
  }
}
class E extends C {
  const E = "E::E";
  protected $np = E::E;
  protected $nq = E::E;
  public $nr = E::E;
  protected $p = 11;
  protected $q = 22;
  public $r = 33;
  function fE() {
    $this->np++;
    $this->nq++;
    $this->nr++;
    $this->p++;
    $this->q++;
    $this->r++;
  }
}
class F extends C {
  const F = "F::F";
  public $np = F::F;
  public $nq = F::F;
  public $nr = F::F;
  public $p = 11;
  public $q = 22;
  public $r = 33;
  function fF() {
    $this->np++;
    $this->nq++;
    $this->nr++;
    $this->p++;
    $this->q++;
    $this->r++;
  }
}
class G extends D {
  const G = "G::G";
  public $np = G::G;
  public $nq = G::G;
  public $nr = G::G;
  public $p = 111;
  public $q = 222;
  public $r = 333;
  function fG() {
    $this->np++;
    $this->nq++;
    $this->nr++;
    $this->p++;
    $this->q++;
    $this->r++;
  }
}
class H {
  const H = "H::H";
  public $np = H::H;
  public $nq = H::H;
  public $nr = H::H;
  public $p = 111;
  public $q = 222;
  public $r = 333;
  function fH() {
    $this->np++;
    $this->nq++;
    $this->nr++;
    $this->p++;
    $this->q++;
    $this->r++;
    $d = new D;
    $d->fC();
    $d->fD();
    $d->r++;
    obj_dump($d);
  }
}
class I {
  const I = "I::I";
  public $p = I::I;
}
class J extends I {
  const J = null;
  public $p = J::J;
}
class ThingerMaker {
  private $refs;
  public function __construct(&$refs) {
    $this->refs =& $refs;
  }
  public function doAssignment() {
    $this->refs = 'it worked';
  }
}

class dumper {
  public function __destruct() {
    var_dump($this);
  }
}
function foo() {
  return new dumper;
}
function useReturn() {
  $five = 5;
  foo()->prop += $five + 5;
}


<<__EntryPoint>>
function main_properties() {
print "Test begin\n";

print "=== C ===\n";
$o = new C;
print $o->nr."\n";
print $o->r."\n";
print $o->s."\n";
obj_dump($o);
$o->fC();
$o->fC();
obj_dump($o);

print "=== D ===\n";
$o = new D;
print $o->nr."\n";
print $o->r."\n";
print $o->s."\n";
$o->fC();
$o->fD();
$o->fC();
$o->fD();
obj_dump($o);

print "=== E ===\n";
$o = new E;
print $o->nr."\n";
print $o->r."\n";
print $o->s."\n";
$o->fC();
$o->fE();
$o->fC();
$o->fE();
obj_dump($o);

print "=== F ===\n";
$o = new F;
print $o->np."\n";
print $o->nq."\n";
print $o->nr."\n";
print $o->p."\n";
print $o->q."\n";
print $o->r."\n";
print $o->s."\n";
$o->fC();
$o->fF();
$o->fC();
$o->fF();
obj_dump($o);

print "=== G ===\n";
$o = new G;
print $o->np."\n";
print $o->nq."\n";
print $o->nr."\n";
print $o->p."\n";
print $o->q."\n";
print $o->r."\n";
print $o->s."\n";
$o->fC();
$o->fD();
$o->fG();
$o->fC();
$o->fD();
$o->fG();
obj_dump($o);

print "=== H ===\n";
$o = new H;
print $o->np."\n";
print $o->nq."\n";
print $o->nr."\n";
print $o->p."\n";
print $o->q."\n";
print $o->r."\n";
$o->fH();
obj_dump($o);

print "=== J ===\n";
$j = new J;
obj_dump($j);

print "=== Var properties ===\n";
$str = "it didn't work";
$d = new ThingerMaker($str);
$d->doAssignment();
echo $str . "\n";

print "=== Foreach ===\n";
foreach ($o as $k => $v) {
  print "  \"".$k."\" => ".$v."\n";
  $o->nr++;
  $o->r++;
}
obj_dump($o);

print "=== Foreach by reference ===\n";
foreach ($o as $k => &$v) {
  print "  \"".$k."\" => ".$v."\n";
  $o->q = "q";
  if ($k == "nr") {
    $v = "nr";
  }
  if ($k == "r") {
    $v = "r";
  }
}
obj_dump($o);
useReturn();

print "Test end\n";
}
