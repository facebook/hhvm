<?php
class Foo {
	var $bar = array();

	static function bar() {
		static $instance = null;
		$instance = new Foo();
		return $instance->bar;
	}
}
extract(Foo::bar());
echo "ok\n";
?>