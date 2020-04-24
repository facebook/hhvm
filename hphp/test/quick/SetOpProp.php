<?hh

class C {
  public $p = 0;
}

class D {
  private $container = darray["a" => "D::a", "b" => 42];
  public $p = 0;
  public function __get($k) {
    print "In D::__get($k)\n";
    return isset($this->container[$k]) ? $this->container[$k] : null;
  }
}

class E {
  private $container = darray["a" => "E::a", "b" => 42];
  public $p = 0;
  public function __get($k) {
    print "In E::__get($k)\n";
    return isset($this->container[$k]) ? $this->container[$k] : null;
  }
  public function __set($k, $v) {
    print "In E::__set($k, $v)\n";
    $this->container[$k] = $v;
  }
}

function error_boundary(inout $x, $fn) {
  try {
    $fn(inout $x);
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
}

<<__EntryPoint>> function main(): void {
print "Test begin\n";

$o = new C;
$o->a .= "<a>";
$o->b .= "<b>";
$o->b .= "<b>";
$o->p += 1;
$o->q += 1;
$o->r .= "hello";
print_r($o);

$o = new D;
$o->a .= "<a>";
$o->b .= "<b>";
$o->b .= "<b>";
$o->p += 1;
$o->q += 1;
$o->r .= "hello";
print_r($o);

$o = new E;
$o->a .= "<a>";
$o->b .= "<b>";
$o->b .= "<b>";
$o->p += 1;
$o->q += 1;
$o->r .= "hello";
print_r($o);

$o = null;
error_boundary(inout $o, (inout $o) ==> $o->a .= "<a>");
error_boundary(inout $o, (inout $o) ==> $o->a .= "<b>");
error_boundary(inout $o, (inout $o) ==> $o->a .= "<b>");
error_boundary(inout $o, (inout $o) ==> $o->a += 1);
error_boundary(inout $o, (inout $o) ==> $o->a += 1);
error_boundary(inout $o, (inout $o) ==> $o->a .= "hello");
print_r($o);

$o = 42;
$o->a .= "<a>";
$o->b .= "<b>";
print_r($o);
print "\n";

print "Test end\n";
}
