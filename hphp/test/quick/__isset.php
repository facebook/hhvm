<?hh

class B {
  private $priv = "priv";
  public $p = 42;
  public function __get($k) {
    print "In C::__get(\"$k\")\n";
    return "__get($k)";
  }
}
class C {
  private $priv = "priv";
  public $p = 42;
}
class D {
  private $priv = "priv";
  public $p = 42;
  public function __isset($k) {
    print "In C::__isset(\"$k\")\n";
    if ($k == "q") {
      return false;
    } else {
      return true;
    }
  }
}
class E {
  private $priv = "priv";
  public $p = 42;
  public function __isset($k) {
    print "In C::__isset(\"$k\")\n";
    if ($k == "q") {
      return false;
    } else {
      return true;
    }
  }
  public function __get($k) {
    print "In C::__get(\"$k\")\n";
    return "__get($k)";
  }
}
<<__EntryPoint>> function main(): void {
print "Test begin\n";
print "--- class B ---\n";
$o = new B();
var_dump(isset($o->priv));
var_dump(isset($o->p));
var_dump(!($o->p ?? false));
var_dump(isset($o->q));
var_dump(!($o->q ?? false));

print "--- class C ---\n";
$o = new C();
var_dump(isset($o->priv));
var_dump(isset($o->p));
var_dump(!($o->p ?? false));
var_dump(isset($o->q));
var_dump(!($o->q ?? false));

print "--- class D ---\n";
$o = new D();
var_dump(isset($o->priv));
var_dump(isset($o->p));
var_dump(!($o->p ?? false));
var_dump(isset($o->q));
var_dump(!($o->q ?? false));
var_dump(isset($o->r));
var_dump(!($o->r ?? false));

print "--- class E ---\n";
$o = new E();
var_dump(isset($o->priv));
var_dump(isset($o->p));
var_dump(!($o->p ?? false));
var_dump(isset($o->q));
var_dump(!($o->q ?? false));
var_dump(isset($o->r));
var_dump(!($o->r ?? false));

print "Test end\n";
}
