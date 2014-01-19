<?php
$strings = array("foo = bar", "bar = foo");
foreach( $strings as $string )
{
	sscanf( $string, "%s = %[^[]]", $var, $val );
	echo "$var = $val\n";
}
?>