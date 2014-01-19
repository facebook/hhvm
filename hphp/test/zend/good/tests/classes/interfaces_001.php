<?php

interface Throwable {
	public function getMessage();
}

class Exception_foo implements Throwable {
	public $foo = "foo";

	public function getMessage() {
		return $this->foo;
	}
}

$foo = new Exception_foo;
echo $foo->getMessage() . "\n";

?>