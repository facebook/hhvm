<?php
class D {
public $prop = 300;
public function prop() { return 350; }
public $yo = 500;
public function yo() { return 550; }
public $blah = 600;
public function blah() { return 650; }
}
class C {
public static $x = 100;
public static $y = array(200);
public static $z = array();
public static function foo1() { return 150; }
public static function foo2() { return 250; }
public $bar1 = 400;
public function bar2() { return 450; }
public $baz;
public $w = array();
public function __construct() {
$this->baz = new D();
$this->w[] = new D();
}
}
C::$z[] = new D();

var_dump(C::$x);
$x = 'foo1';
var_dump(C::$x());

var_dump(C::$y[0]);
$y = array('foo2');
var_dump(C::$y[0]());

var_dump(C::$z[0]->prop);
var_dump(C::$z[0]->prop());

$obj = new C;
$x = array('bar1');
var_dump($obj->$x[0]);
$x = array('bar2');
var_dump($obj->$x[0]());

$obj = new C;
var_dump($obj->w[0]->yo);
var_dump($obj->w[0]->yo());

$obj = new C;
$w = array('baz');
var_dump($obj->$w[0]->blah);
var_dump($obj->$w[0]->blah());
