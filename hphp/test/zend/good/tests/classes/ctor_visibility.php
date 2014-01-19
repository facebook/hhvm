<?php

class Test
{
    function __construct()
    {
        echo __METHOD__ . "()\n";
    }
}

class Derived extends Test
{
	function __construct()
	{
        echo __METHOD__ . "()\n";
		parent::__construct();
	}
	
	static function f()
	{
		new Derived;
	}
}

Derived::f();

class TestPriv
{
    private function __construct()
    {
        echo __METHOD__ . "()\n";
    }

	static function f()
	{
		new TestPriv;
	}
}

TestPriv::f();

class DerivedPriv extends TestPriv
{
	function __construct()
	{
        echo __METHOD__ . "()\n";
		parent::__construct();
	}
	
	static function f()
	{
		new DerivedPriv;
	}
}

DerivedPriv::f();

?>
===DONE===