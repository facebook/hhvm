<?php

class C {
  private $priv;
  public $pub;
  public function __set($k, $v) {
    echo "set $k $v\n";
    $this->$k = $v;
  }
  public function __get($k) {
    echo "get $k\n";
    return $this->$k;
  }
}

$obj = new C();

// These are extension functions that call into o_set() and o_get().  These
// exercise t___get() and t___set().
hphp_set_property($obj, null, "priv", 1234);
var_dump(hphp_get_property($obj, "unused arg", "priv"));
hphp_set_property($obj, null, "pub", 123);
var_dump(hphp_get_property($obj, "unused arg", "pub"));
var_dump($obj);

$obj = new C();

// These exercise direct property setting and getting.
$obj->priv = 6789;
var_dump($obj->priv);
$obj->pub = 678;
var_dump($obj->pub);
var_dump($obj);

echo "\n******************************\n\n";

class D {
  protected $foo;
  public function __get($k) { echo "__get $k\n"; }
  public function __set($k, $v) { echo "__set $k\n"; }
}
$obj = new D;
$obj->foo = 1;
var_dump($obj);
$obj = null;

$obj = new D;
$obj->foo += 1;
var_dump($obj);
$obj = null;

$obj = new D;
$obj->foo++;
var_dump($obj);
$obj = null;

echo "\n******************************\n\n";

class E {
  public function __get($k) {
    echo "__get $k\n";
  }
}

$obj = new E;
$obj->foo = 1;
var_dump($obj);
$obj = null;

$obj = new E;
$obj->foo += 1;
var_dump($obj);
$obj = null;

$obj = new E;
$obj->foo++;
var_dump($obj);
$obj = null;

echo "\n******************************\n\n";

class F {
  public function __set($k, $v) {
    echo "__set $k\n";
  }
}

$obj = new F;
$obj->foo = 1;
var_dump($obj);
$obj = null;

$obj = new F;
$obj->foo += 1;
var_dump($obj);
$obj = null;

$obj = new F;
$obj->foo++;
var_dump($obj);
$obj = null;

