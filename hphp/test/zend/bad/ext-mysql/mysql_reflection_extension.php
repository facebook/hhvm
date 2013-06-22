<?php
	$r = new ReflectionExtension("mysql");

	printf("Name: %s\n", $r->name);
	printf("Version: %s\n", $r->getVersion());
	$classes = $r->getClasses();
	if (!empty($classes)) {
		printf("[002] Expecting no class\n");
		asort($classes);
		var_dump($classes);
	}

	$ignore = array();

	$functions = $r->getFunctions();
	asort($functions);
	printf("Functions:\n");
	foreach ($functions as $func) {
		if (isset($ignore[$func->name])) {
			unset($ignore[$func->name]);
		} else {
			printf("  %s\n", $func->name);
		}
	}
	if (!empty($ignore)) {
		printf("Dumping version dependent and missing functions\n");
		var_dump($ignore);
	}


	print "done!";
?>