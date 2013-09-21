<?php
class A {
	use T2 { t2method as Bmethod; }
}

class B extends A {
}

trait T2 {
	public function t2method() {
	}
}
print_r(get_class_methods("A"));
print_r(get_class_methods("B"));