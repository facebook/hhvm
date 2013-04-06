<?php

interface Throwable {
	public function getMessage();
	public function getErrno();
}

class Exception_foo implements Throwable {
	public $foo = "foo";

	public function getMessage() {
		return $this->foo;
	}
}

// this should die -- Exception class must be abstract...
$foo = new Exception_foo;
echo "Message: " . $foo->getMessage() . "\n";

?>
===DONE===