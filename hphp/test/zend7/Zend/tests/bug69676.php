<?php
class A {
	const myConst = "const in A";
	const myDynConst = self::myConst;
}

class B extends A {
	const myConst = "const in B";
}

var_dump(B::myDynConst);
var_dump(A::myDynConst);
?>
