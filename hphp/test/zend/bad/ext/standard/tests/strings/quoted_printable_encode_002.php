<?php

$s = str_repeat("\0", 200);
var_dump($d = quoted_printable_encode($s));
var_dump(quoted_printable_decode($d));

$s = str_repeat("строка в юникоде", 50);
var_dump($d = quoted_printable_encode($s));
var_dump(quoted_printable_decode($d));

class foo {
	function __toString() {
		return "this is a foo";
	}
}

$o = new Foo;
var_dump(quoted_printable_encode($o));

echo "Done\n";
?>