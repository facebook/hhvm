<?php
class DestructableObject
{
	public function __destruct()
	{
		echo "2\n";
	}	
}

class DestructorCreator
{
	public function __destruct()
	{
		$this->test = new DestructableObject;	
		echo "1\n";
	}
}

class Test
{
	public static $mystatic;
}

// Uncomment this to avoid segfault
//Test::$mystatic = new DestructorCreator();

$x = new Test();

if (!isset(Test::$mystatic))
	Test::$mystatic = new DestructorCreator();

echo "bla\n";
?>