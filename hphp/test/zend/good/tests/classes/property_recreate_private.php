<?php
class C {
	private $p = 'test';
	function unsetPrivate() {
		unset($this->p);		
	}
	function setPrivate() {
		$this->p = 'changed';		
	}
}

class D extends C {
	function setP() {
		$this->p = 'changed in D';
	}
}

echo "Unset and recreate a superclass's private property:\n";
$d = new D;
$d->unsetPrivate();
$d->setPrivate();
var_dump($d);

echo "\nUnset superclass's private property, and recreate it as public in subclass:\n";
$d = new D;
$d->unsetPrivate();
$d->setP();
var_dump($d);

echo "\nUnset superclass's private property, and recreate it as public at global scope:\n";
$d = new D;
$d->unsetPrivate();
$d->p = 'this will create a public property';
var_dump($d);


echo "\n\nUnset and recreate a private property:\n";
$c = new C;
$c->unsetPrivate();
$c->setPrivate();
var_dump($c);

echo "\nUnset a private property, and attempt to recreate at global scope (expecting failure):\n";
$c = new C;
$c->unsetPrivate();
$c->p = 'this will fail';
var_dump($c);
?>
==Done==
