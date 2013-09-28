<?php

class A {
 private $a;
 protected $b;
 public $c;
 static $d;
 }
function f($a) {
 asort($a);
 foreach ($a as $v) {
 var_dump($v->getName());
 }
 }
$r = new ReflectionClass('A');
$a = $r->getProperties();
 f($a);
$a = $r->getProperties(ReflectionProperty::IS_PUBLIC);
 f($a);
$a = $r->getProperties(ReflectionProperty::IS_PRIVATE);
 f($a);
$a = $r->getProperties(ReflectionProperty::IS_PROTECTED);
 f($a);
$a = $r->getProperties(ReflectionProperty::IS_STATIC);
 f($a);
