<?php
class MyAutoloader {
	function __construct($directory_to_use) {}
	function autoload($class_name) {
		// code to autoload based on directory
	}
}

$autloader1 = new MyAutoloader('dir1');
spl_autoload_register(array($autloader1, 'autoload'));

$autloader2 = new MyAutoloader('dir2');
spl_autoload_register(array($autloader2, 'autoload'));

print_r(spl_autoload_functions());
?>
===DONE===