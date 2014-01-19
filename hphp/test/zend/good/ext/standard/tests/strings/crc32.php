<?php
$input = array("foo", "bar", "baz", "grldsajkopallkjasd");
foreach($input AS $i) {
	printf("%u\n", crc32($i));
}
?>