<?hh

class A {
 private $a;
 protected $b;
 public $c;
 static $d;
 }
function f($a) {
 asort(inout $a);
 foreach ($a as $v) {
 var_dump($v->getName());
 }
 }

<<__EntryPoint>>
function main_1362() {
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
}
