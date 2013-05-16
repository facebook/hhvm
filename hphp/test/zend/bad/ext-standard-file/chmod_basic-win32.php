<?php

define("PERMISSIONS_MASK", 0777);

$filename = __FILE__ . ".tmp";

$fd = fopen($filename, "w+");
fclose($fd);

for ($perms_to_set = 0777; $perms_to_set >= 0; $perms_to_set--) {
	chmod($filename, $perms_to_set);
	$set_perms = (fileperms($filename) & PERMISSIONS_MASK);
	clearstatcache();
	printf("Setting mode %o gives mode %o\n", $perms_to_set, $set_perms);
}
var_dump(chmod($filename, 0777));

unlink($filename);
echo "done";

?>