<?php
abstract class Foo {
        abstract static function bar();
}
interface MyInterface {
    static function bar();
}
abstract class Bar {
	static function foo() {
		echo "ok\n";
	}
}
var_dump(is_callable(array("Foo", "bar")));
var_dump(is_callable("Foo::bar"));
var_dump(is_callable(array("MyInterface", "bar")));
var_dump(is_callable("MyInterface::bar"));
var_dump(is_callable(array("Bar", "foo")));
var_dump(is_callable("Bar::foo"));
Bar::foo();
Foo::bar();
?>