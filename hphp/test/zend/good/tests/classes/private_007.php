<?php

class Bar {
	public static function pub() {
		Bar::priv();
	}
	private static function priv()	{
		echo "Bar::priv()\n";
	}
}
class Foo extends Bar {
	public static function priv()	{ 
		echo "Foo::priv()\n";
	}
}

Foo::pub();
Foo::priv();

echo "Done\n";
?>
