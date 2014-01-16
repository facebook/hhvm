<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;

@unlink($fname);

file_put_contents($pname . '/a.php?', "query");
file_put_contents($pname . '/b.php?bla', "query");

var_dump(file_get_contents($pname . '/a.php'));
var_dump(file_get_contents($pname . '/b.php'));

function error_handler($errno, $errmsg)
{
	echo "Error: $errmsg\n";
}

set_error_handler('error_handler');

$checks = array('/', '.', '../', 'a/..', 'a/', 'b//a.php');
foreach($checks as $check)
{
	file_put_contents($pname . '/' . $check, "error");
}

$phar = new Phar($fname);
$checks = array("a\0");
foreach($checks as $check)
{
	try
	{
		$phar[$check] = 'error';
	}
	catch(Exception $e)
	{
		echo 'Exception: ' . $e->getMessage() . "\n";
	}
}

?>
===DONE===
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php'); ?>