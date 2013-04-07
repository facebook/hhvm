<?php

class Root {
}

interface MyInterface
{
	static function MyInterfaceFunc();
}

abstract class Derived extends Root implements MyInterface {
}

class Leaf extends Derived
{
	static function MyInterfaceFunc() {}	
}

var_dump(new Leaf);

class Fails extends Root implements MyInterface {
}

?>
===DONE===