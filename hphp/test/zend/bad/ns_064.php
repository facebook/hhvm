<?php

namespace test;

class foo {
	public $e = array();
	
	public function __construct() {
		$this->e[] = $this;
	}
	
	public function __set($a, $b) {
		var_dump($a, $b);
	}
	public function __get($a) {
		var_dump($a);
		return $this;
	}
}

use test\foo as stdClass;

$x = new stdClass;
$x->a = 1;
$x->b->c = 1;
$x->d->e[0]->f = 2;

?>