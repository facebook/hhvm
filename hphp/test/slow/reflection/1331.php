<?hh

class B {
 public function f($a) {
 return 'ok'.$a;
}
}
 class A extends B {
 public $p = 'g';
}
 $obj = new A();
 var_dump(get_class($obj));
 var_dump(get_parent_class($obj));
 var_dump(is_a($obj, 'b'));
 var_dump(is_subclass_of($obj, 'b'));
var_dump(method_exists($obj, 'f'));
var_dump(method_exists('A', 'f'));
var_dump(is_callable(array($obj, 'f')));
var_dump(is_callable(array('A', 'f')));
var_dump(get_object_vars($obj));
var_dump(call_user_method('f', $obj, 'blah'));
var_dump(call_user_method_array('f', $obj, array('blah')));
var_dump(call_user_method_array('f', $obj, Vector {'blah'}));
