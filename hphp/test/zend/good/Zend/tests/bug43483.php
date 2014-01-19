<?php
class C {
	public static function test() {
		D::prot();
		print_r(get_class_methods("D"));
	}
}
class D extends C {
	protected static function prot() {
		echo "Successfully called D::prot().\n";
	}
}
D::test();
?>