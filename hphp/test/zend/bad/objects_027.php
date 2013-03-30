<?php 

class bar {
	public function show() {
		var_dump(new static);
	}
}
 
class foo extends bar {
	public function test() {
		parent::show();
	}
}
 
$foo = new foo;
$foo->test();
$foo::test();

call_user_func(array($foo, 'test'));
call_user_func(array('foo', 'test'));

?>