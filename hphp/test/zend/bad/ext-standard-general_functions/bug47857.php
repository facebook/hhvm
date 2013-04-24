<?php
class foo {
	function bar() {
		echo "ok\n";
	}
}
var_dump(is_callable(array('foo','bar')));
foo::bar();
var_dump(is_callable(array('Exception','getMessage')));
Exception::getMessage();
?>