<?php
	
	class foo {
	}
	$foo = new foo();
	$foo->abc = 'def';
	
	$string = wddx_serialize_value($foo);
	$bar = wddx_deserialize($string);

	echo "OK\n";

?>
