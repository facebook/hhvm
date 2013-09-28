<?php

function __autoload($className) {
	var_dump($className);
	
	if ($className == 'Foo') {
		class Foo implements Bar {};
	} else {
		throw new Exception($className);
	}
}

new Foo;

?>