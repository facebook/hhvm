<?php

class Foo {
	public $name;
    
	function Foo() {
		$this->name = "I'm Foo!\n";
	}
}

$foo = new Foo;
echo $foo->name;
$bar = $foo;
$bar->name = "I'm Bar!\n";

// In ZE1, we would expect "I'm Foo!"
echo $foo->name;

?>
