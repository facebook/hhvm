<?php

print "Test begin\n";

print "--- class B ---\n";
class B {
  private $priv = "priv";
  public $p = 42;
  public function __get($k) {
    print "In C::__get(\"$k\")\n";
    return "__get($k)";
  }
}
$o = new B();
var_dump(isset($o->priv));
var_dump(empty($o->priv));
var_dump(isset($o->p));
var_dump(empty($o->p));
var_dump(isset($o->q));
var_dump(empty($o->q));

print "--- class C ---\n";
class C {
  private $priv = "priv";
  public $p = 42;
}
$o = new C();
var_dump(isset($o->priv));
var_dump(empty($o->priv));
var_dump(isset($o->p));
var_dump(empty($o->p));
var_dump(isset($o->q));
var_dump(empty($o->q));

print "--- class D ---\n";
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
$o = new D();
var_dump(isset($o->priv));
var_dump(empty($o->priv));
var_dump(isset($o->p));
var_dump(empty($o->p));
var_dump(isset($o->q));
var_dump(empty($o->q));
var_dump(isset($o->r));
var_dump(empty($o->r));

print "--- class E ---\n";
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
$o = new E();
var_dump(isset($o->priv));
var_dump(empty($o->priv));
var_dump(isset($o->p));
var_dump(empty($o->p));
var_dump(isset($o->q));
var_dump(empty($o->q));
var_dump(isset($o->r));
var_dump(empty($o->r));

print "Test end\n";
