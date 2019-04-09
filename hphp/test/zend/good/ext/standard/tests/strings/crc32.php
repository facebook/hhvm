<?php
$input = array("foo", "bar", "baz", "grldsajkopallkjasd");
foreach($input as $i) {
	printf("%u\n", crc32($i));
}
