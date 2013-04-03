<?php
Class A {
	private $c;
}

Class B extends A {
	private $c;
}

Class C extends B {
}

var_dump(new C);
?>