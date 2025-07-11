<?hh
<<__DynamicallyCallable>> function id($x = 'id') :mixed{ return $x; }
class Test {
  public static function id($x = vec[__CLASS__, 'id']) :mixed{ return $x; }
  public static $f;
}

<<__EntryPoint>>
function main_indirect_fcall() :mixed{
var_dump(0);
HH\dynamic_fun(id('var_dump'))(1);
HH\dynamic_fun(HH\dynamic_fun(id('id'))('var_dump'))(2);
HH\dynamic_fun(HH\dynamic_fun(HH\dynamic_fun(id('id'))('id'))('var_dump'))(3);
HH\dynamic_fun(HH\dynamic_fun(HH\dynamic_fun(id())())('var_dump'))(4);
HH\dynamic_fun(HH\dynamic_fun(HH\dynamic_fun(id(vec['udef', 'id'])[1])())('var_dump'))(5);
$o = new stdClass(); $o->a = 'id'; $o->b = 'udef';
HH\dynamic_fun(HH\dynamic_fun(HH\dynamic_fun(HH\dynamic_fun(HH\dynamic_fun(HH\dynamic_fun((id($o)->a))())())())())('var_dump'))(6);
$id = function($x) { return $x; };
HH\dynamic_fun($id($id)('var_dump'))(7);
HH\dynamic_fun(HH\dynamic_fun((function($x) { return $x; })('id'))('var_dump'))(8);
HH\dynamic_fun((Test::$f = function($x = null) {
    return $x ?: Test::$f;
})()()()('var_dump'))(9);
$obj = new Test;
HH\dynamic_fun(HH\dynamic_fun(vec[$obj, 'id']()('id'))($id)('var_dump'))(10);
HH\dynamic_fun(vec['Test', 'id']()()('var_dump'))(11);
HH\dynamic_fun(HH\dynamic_fun(HH\dynamic_fun(HH\dynamic_fun('id')())('id'))('var_dump'))(12);
HH\dynamic_fun(HH\dynamic_fun(HH\dynamic_fun(('i' . 'd'))())('var_dump'))(13);
}
