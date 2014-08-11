<?php

class foo { 
}

$x = new foo;
$x->test = 2000;


$p = new ReflectionObject($x);
var_dump($p->getProperty('test'));


class bar {
	public function __construct() {
		$this->a = 1;
	}
}

class test extends bar {
	private $b = 2;

	public function __construct() {
		parent::__construct();
		
		$p = new reflectionobject($this);
		var_dump($h = $p->getProperty('a'));
		var_dump($h->isDefault(), $h->isProtected(), $h->isPrivate(), $h->isPublic(), $h->isStatic());
		var_dump($p->getProperties());
	}
}

new test;

?>
===DONE===
