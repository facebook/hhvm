<?php
$bar = NULL;
class bad {
	public function __destruct() {
		global $bar;
		$bar = $this;
		$bar->y = new stdClass;
	}
}

$foo = new stdClass;
$foo->foo = $foo;
$foo->bad = new bad;
$foo->bad->x = new stdClass;

unset($foo);
gc_collect_cycles();
var_dump($bar);
