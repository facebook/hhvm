<?php

class Foo {
	public function __call($a, $b) {
		print "nonstatic\n";
		var_dump($a);
	}
}

$a = new Foo;
call_user_func(array($a, 'aAa'));

