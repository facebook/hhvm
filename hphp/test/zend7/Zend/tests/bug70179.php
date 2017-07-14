<?php

class X {
	function __invoke() {
		var_dump($this);
	}
}
(new X)();

?>
