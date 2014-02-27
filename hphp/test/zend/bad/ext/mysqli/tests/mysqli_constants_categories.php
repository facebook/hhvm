<?php
	$constants = get_defined_constants(true);

	foreach ($constants as $group => $consts) {
		foreach ($consts as $name => $value) {
			if (stristr($name, 'mysqli')) {
				if ('mysqli' != $group)
				printf("found constant '%s' in group '%s'. expecting group 'mysqli'\n", $name, $group);
			}
		}
	}

	print "done!";
?>