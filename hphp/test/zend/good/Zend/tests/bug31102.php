<?php

$test = 0;

function __autoload($class)
{
	global $test;

	echo __METHOD__ . "($class,$test)\n";
	switch($test)
	{
	case 1:
		eval("class $class { function __construct(){throw new Exception('$class::__construct');}}");
		return;
	case 2:
		eval("class $class { function __construct(){throw new Exception('$class::__construct');}}");
		throw new Exception(__METHOD__);
		return;
	case 3:
		return;
	}
}

while($test++ < 5)
{
	try
	{
		eval("\$bug = new Test$test();");
	}
	catch (Exception $e)
	{
		echo "Caught: " . $e->getMessage() . "\n";
	}
}
?>
===DONE===
<?php exit(0); ?>