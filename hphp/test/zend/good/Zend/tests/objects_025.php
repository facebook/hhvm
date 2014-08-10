<?php 

class foo {
	public function __call($a, $b) {
		print "non-static - ok\n";
	}
	
	public static function __callstatic($a, $b) {
		print "static - ok\n";
	}
}

$a = new foo;
$a->foooo();
$a::foooo();

$b = 'aaaaa1';
$a->$b();
$a::$b();

$b = '  ';
$a->$b();
$a::$b();

$b = str_repeat('a', 10000);
$a->$b();
$a::$b();

$b = NULL;
$a->$b();

?>
