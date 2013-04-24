<?php

// This test also needs to exclude the protected and private variables 
// since they cannot be accessed from the external object which iterates 
// them.

class test
{
	public    $pub = "public";
	protected $pro = "protected";
	private   $pri = "private";
	
	function __construct()
	{
		$this->imp = "implicit";
	}
};

$test = new test;
$test->dyn = "dynamic";

print_r($test);

$object = new ArrayObject($test);

print_r($object);

foreach($test as $key => $val)
{
	echo "$key => $val\n";
}

?>
===DONE===
<?php exit(0); ?>