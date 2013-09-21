<?php
	$r = new ReflectionExtension("mysql");

	$ignore = array();

	$functions = $r->getFunctions();
	asort($functions);
	printf("Functions:\n");
	foreach ($functions as $func) {
		if (isset($ignore[$func->name]))
			continue;

		printf("  %s\n", $func->name);
		$rf = new ReflectionFunction($func->name);
		printf("    Deprecated: %s\n", $rf->isDeprecated() ? "yes" : "no");
		printf("    Accepted parameters: %d\n", $rf->getNumberOfParameters());
		printf("    Required parameters: %d\n", $rf->getNumberOfRequiredParameters());
		foreach( $rf->getParameters() as $param ) {
			printf("      %s\n", $param);
		}
	}

	print "done!";
?>