<?php

function MyAutoLoad($className)
{
	echo __METHOD__ . "($className)\n";
	throw new Exception('Bla');
}

class MyAutoLoader
{
	static function autoLoad($className)
	{
		echo __METHOD__ . "($className)\n";
		throw new Exception('Bla');
	}
	
	function dynaLoad($className)
	{
		echo __METHOD__ . "($className)\n";
		throw new Exception('Bla');
	}
}

$obj = new MyAutoLoader;

$funcs = array(
	'MyAutoLoad',
	'MyAutoLoader::autoLoad',
	'MyAutoLoader::dynaLoad',
	array('MyAutoLoader', 'autoLoad'),
	array('MyAutoLoader', 'dynaLoad'),
	array($obj, 'autoLoad'),
	array($obj, 'dynaLoad'),
);

foreach($funcs as $idx => $func)
{
	echo "====$idx====\n";

	try
	{
		var_dump($func);
		spl_autoload_register($func);
		if (count(spl_autoload_functions()))
		{
			echo "registered\n";
			
			var_dump(class_exists("NoExistingTestClass", true));
		}		
	}
	catch (Exception $e)
	{
		echo get_class($e) . ": " . $e->getMessage() . "\n";
	}

	spl_autoload_unregister($func);
	var_dump(count(spl_autoload_functions()));
}

?>
===DONE===
<?php exit(0); ?>