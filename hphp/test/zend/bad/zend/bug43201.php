<?php
class Foo {
	function __get($k) {
		return null;
	}
	function __set($k, $v) {
		$this->$k = $v;
	}
}

$c = new Foo();

$c->arr[0]["k"] = 1;
$c->arr[0]["k2"] = $ref;
for($cnt=0;$cnt<6;$cnt++) {
	$ref = chop($undef);	
	$c->arr[$cnt]["k2"] = $ref;
}
echo "ok\n";
?>