<?php
class A {
	const myConst = "const in A";
	const myDynConst = self::myConst;
	
	public static function test() {
		var_dump(self::myDynConst);
	}
}

class B extends A {
	const myConst = "const in B";

	public static function test() {
		var_dump(parent::myDynConst);
	}
}

B::test();
A::test();
?>