<?php
class A {
	private $c;
}

class B extends A {
	private $c;
}

class C extends B {
}

var_dump(new C);
