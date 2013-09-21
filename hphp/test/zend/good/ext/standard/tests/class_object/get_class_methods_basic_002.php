<?php
/* Prototype  : proto array get_class_methods(mixed class)
 * Description: Returns an array of method names for class or class instance. 
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions: 
 */

/*
 * Test behaviour with various visibility levels.
 */

class C {
	private function privC() {}
	protected function protC() {}
	public function pubC() {}
	
	public static function testFromC() {
		echo "Accessing C from C:\n";
		var_dump(get_class_methods("C"));
		echo "Accessing D from C:\n";
		var_dump(get_class_methods("D"));
		echo "Accessing X from C:\n";
		var_dump(get_class_methods("X"));
	}
}

class D extends C {
	private function privD() {}
	protected function protD() {}
	public function pubD() {}
	
	public static function testFromD() {
		echo "Accessing C from D:\n";
		var_dump(get_class_methods("C"));
		echo "Accessing D from D:\n";
		var_dump(get_class_methods("D"));
		echo "Accessing X from D:\n";
		var_dump(get_class_methods("X"));
	}
}

class X {
	private function privX() {}
	protected function protX() {}
	public function pubX() {}
	
	public static function testFromX() {
		echo "Accessing C from X:\n";
		var_dump(get_class_methods("C"));
		echo "Accessing D from X:\n";
		var_dump(get_class_methods("D"));
		echo "Accessing X from X:\n";
		var_dump(get_class_methods("X"));
	}
}

echo "Accessing D from global scope:\n";
var_dump(get_class_methods("D"));

C::testFromC();
D::testFromD();
X::testFromX();

echo "Done";
?>