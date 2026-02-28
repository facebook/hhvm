<?hh

class B {
 public function f($a) :mixed{
 return 'ok'.$a;
}
}
 class A extends B {
 public $p = 'g';
}

 <<__EntryPoint>>
function main_1331() :mixed{
$obj = new A();
 var_dump(get_class($obj));
 var_dump(get_parent_class($obj));
 var_dump(is_a($obj, 'B'));
 var_dump(is_subclass_of($obj, 'B'));
var_dump(method_exists($obj, 'f'));
var_dump(method_exists('A', 'f'));
var_dump(is_callable(vec[$obj, 'f']));
var_dump(is_callable(vec['A', 'f']));
var_dump(get_object_vars($obj));
}
