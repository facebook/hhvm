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

foreach(array(1=>"foo",2=>42) as $key => $value)
{
     $storage->attach(new TestClass($key), $value);
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
$storage->attach(new TestClass(3), new stdClass);
$storage->attach(new TestClass(4), new TestClass(5));
echo "===UNSERIALIZE2===\n";
var_dump(unserialize(serialize($storage)));
$storage->rewind();
$storage->next();
var_dump($storage->key());
var_dump($storage->current());
var_dump($storage->getInfo());
$storage->setInfo("bar");
var_dump($storage->getInfo());
echo "===UNSERIALIZE3===\n";
var_dump(unserialize(serialize($storage)));
$storage->rewind();
$storage->next();
$storage->next();
var_dump($storage->key());
var_dump($storage->current());
$storage->attach($storage->current(), "replaced");
echo "===UNSERIALIZE4===\n";
var_dump(unserialize(serialize($storage)));

?>
===DONE===
<?php exit(0); ?>