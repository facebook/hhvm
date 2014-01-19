<?php
class MyClass
{
	static public function loadCode($p) {
		return include $p;
	}
}

MyClass::loadCode('file-which-does-not-exist-on-purpose.php');