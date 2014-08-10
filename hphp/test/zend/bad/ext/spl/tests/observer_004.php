<?php

class TestClass
{
	public $test = 25;
	
	public function __construct($test = 42)
	{
		$this->test = $test;
	}
}

class MyStorage extends SplObjectStorage
{
	public $bla = 25;
	
	public function __construct($bla = 26)
	{
		$this->bla = $bla;
	}
}

$storage = new MyStorage();

foreach(array(1,2) as $value)
{
     $storage->attach(new TestClass($value));
}

var_dump(count($storage));

foreach($storage as $object)
{
	var_dump($object->test);
}

var_dump($storage);

var_dump(serialize($storage));
echo "===UNSERIALIZE===\n";

$storage2 = unserialize(serialize($storage));

var_dump(count($storage2));

foreach($storage2 as $object)
{
	var_dump($object->test);
}

var_dump($storage2);

?>
===DONE===
<?php exit(0); ?>
