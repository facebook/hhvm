<?php

class A {
	static protected function f() {return 'A::f()';}
}
class B1 extends A {
	static protected function f() {return 'B1::f()';}
}
class B2 extends A {
	static public function test() {echo B1::f();}
}
B2::test();

?>