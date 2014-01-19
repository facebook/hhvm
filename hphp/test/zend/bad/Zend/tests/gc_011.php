<?php
class Foo {
	public $a;
	function __destruct() {
		echo __FUNCTION__,"\n";
	}
}
$a = new Foo();
$a->a = $a;
var_dump($a);
unset($a);
var_dump(gc_collect_cycles());
echo "ok\n"
?>