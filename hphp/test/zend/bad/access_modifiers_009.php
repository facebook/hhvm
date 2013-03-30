<?php

class A {
	static protected function f() {return 'A::f()';}
}
class B1 extends A {
	static protected function f() {return 'B1::f()';}
}
class B2 extends A {
	static public function test() {
		var_dump(is_callable('B1::f'));
		B1::f();
	}
}
B2::test();

?>