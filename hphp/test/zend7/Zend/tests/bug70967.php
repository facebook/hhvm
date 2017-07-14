<?php
class A {
	public function __toString() {
		undefined_function();
	}
}

echo (new A);
?>
