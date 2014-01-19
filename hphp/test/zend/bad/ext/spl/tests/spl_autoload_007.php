<?php

class MyAutoLoader {

        static protected function noAccess($className) {
        	echo __METHOD__ . "($className)\n";
        }

        static function autoLoad($className) {
        	echo __METHOD__ . "($className)\n";
        }

        function dynaLoad($className) {
        	echo __METHOD__ . "($className)\n";
        }
}

$obj = new MyAutoLoader;

$funcs = array(
	'MyAutoLoader::notExist',
	'MyAutoLoader::noAccess',
	'MyAutoLoader::autoLoad',
	'MyAutoLoader::dynaLoad',
	array('MyAutoLoader', 'notExist'),
	array('MyAutoLoader', 'noAccess'),
	array('MyAutoLoader', 'autoLoad'),
	array('MyAutoLoader', 'dynaLoad'),
	array($obj, 'notExist'),
	array($obj, 'noAccess'),
	array($obj, 'autoLoad'),
	array($obj, 'dynaLoad'),
);

foreach($funcs as $idx => $func)
{
	if ($idx) echo "\n";
	try
	{
		var_dump($func);
		spl_autoload_register($func);
		echo "ok\n";
	}
	catch (Exception $e)
	{
		echo $e->getMessage() . "\n";
	}
}

?>
===DONE===
<?php exit(0); ?>