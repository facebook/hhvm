<?php

class foo {
	static public function a() {
		print "ok\n";
	}
}

$a = 'a';
$b = 'a';

$class = 'foo';

foo::a();
foo::$a();

$class::a();
$class::$a();

