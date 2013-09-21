<?php
class foo {
	function __destruct() {
		foreach ($this->x as $x);
	}
}
new foo();
echo 'OK';
?>