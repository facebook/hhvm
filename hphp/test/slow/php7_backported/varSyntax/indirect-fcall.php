<?hh
function id($x = 'id') { return $x; }
class Test {
  public static function id($x = varray[__CLASS__, 'id']) { return $x; }
  public static $f;
}

<<__EntryPoint>>
function main_indirect_fcall() {
var_dump(0);
id('var_dump')(1);
id('id')('var_dump')(2);
id('id')('id')('var_dump')(3);
id()()('var_dump')(4);
id(varray['udef', 'id'])[1]()('var_dump')(5);
$o = new stdClass(); $o->a = 'id'; $o->b = 'udef';
(id($o)->a)()()()()('var_dump')(6);
$id = function($x) { return $x; };
$id($id)('var_dump')(7);
(function($x) { return $x; })('id')('var_dump')(8);
(Test::$f = function($x = null) {
    return $x ?: Test::$f;
})()()()('var_dump')(9);
$obj = new Test;
varray[$obj, 'id']()('id')($id)('var_dump')(10);
varray['Test', 'id']()()('var_dump')(11);
'id'()('id')('var_dump')(12);
('i' . 'd')()('var_dump')(13);
}
