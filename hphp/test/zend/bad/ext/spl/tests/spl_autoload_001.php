<?php

echo "===EMPTY===\n";

var_dump(spl_autoload_extensions());

try
{
	spl_autoload("TestClass");
}
catch(Exception $e)
{
	echo 'Exception: ' . $e->getMessage() . "\n";
}

$test_exts = array(NULL, "1", ".inc,,.php.inc", "");

foreach($test_exts as $exts)
{
	echo "===($exts)===\n";
	try
	{
		spl_autoload("TestClass", $exts);
	}
	catch(Exception $e)
	{
		echo 'Exception: ' . $e->getMessage() . "\n";
	}
}

try
{
	spl_autoload_extensions(".inc,.php.inc");
	spl_autoload("TestClass");
}
catch(Exception $e)
{
	echo 'Exception: ' . $e->getMessage() . "\n";
}

function TestFunc1($classname)
{
	echo __METHOD__ . "($classname)\n";
}

function TestFunc2($classname)
{
	echo __METHOD__ . "($classname)\n";
}

echo "===SPL_AUTOLOAD()===\n";

spl_autoload_register();

try
{
	var_dump(spl_autoload_extensions(".inc"));
	var_dump(class_exists("TestClass", true));
}
catch(Exception $e)
{
	echo 'Exception: ' . $e->getMessage() . "\n";
}

echo "===REGISTER===\n";

spl_autoload_unregister("spl_autoload");
spl_autoload_register("TestFunc1");
spl_autoload_register("TestFunc2");
spl_autoload_register("TestFunc2"); /* 2nd call ignored */
spl_autoload_extensions(".inc,.class.inc"); /* we do not have spl_autoload_registered yet */

try
{
	var_dump(class_exists("TestClass", true));
}
catch(Exception $e)
{
	echo 'Exception: ' . $e->getMessage() . "\n";
}

echo "===LOAD===\n";

spl_autoload_register("spl_autoload");
var_dump(class_exists("TestClass", true));

echo "===NOFUNCTION===\n";

try
{
	spl_autoload_register("unavailable_autoload_function");
}
catch(Exception $e)
{
	echo 'Exception: ' . $e->getMessage() . "\n";
}

?>
===DONE===
<?php exit(0); ?>