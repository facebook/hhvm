<?php
	$foo = create_function('$s', 'return strtoupper($s);');
	ob_start($foo);
	echo $foo("bar\n");
?>
bar