<?php
require_once __DIR__."/../utils/server.inc";
$a = mongo_standalone();
$names = array("\\", "\$", "/", "foo.bar");
foreach ($names as $name) {
	try {
		$d = new MongoDB($a, $name);
	} catch (Exception $e) {
		echo $name, ": ", $e->getMessage(), "\n";
	}
}
?>