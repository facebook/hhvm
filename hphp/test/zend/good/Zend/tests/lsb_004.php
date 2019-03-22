<?php

class TestClass {
	public static function getClassName() {
		return static::class;
	}
}

class ChildClass extends TestClass {}

echo TestClass::getClassName() . "\n";
echo ChildClass::getClassName() . "\n";
echo "==DONE==";
