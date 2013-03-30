<?php

class TestClass {
	public static function getClassName() {
		return get_called_class();
	}
}

class ChildClass extends TestClass {}

echo TestClass::getClassName() . "\n";
echo ChildClass::getClassName() . "\n";
?>
==DONE==