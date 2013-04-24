<?php
include_once __DIR__ . '/common.inc';
fix_acls();

$iteration = array(
	PHPT_ACL_READ => true,
	PHPT_ACL_NONE => false,
	PHPT_ACL_WRITE => false,
	PHPT_ACL_WRITE|PHPT_ACL_READ => true,
);

echo "Testing file:\n";
$i = 1;
$path = __DIR__ . '/a.txt';
foreach ($iteration as $perms => $exp) {
	create_file($path, $perms);
	clearstatcache(true, $path);
	echo 'Iteration #' . $i++ . ': ';
	if (is_readable($path) == $exp) {
		echo "passed.\n";
	} else {
		var_dump(is_readable($path), $exp);
		echo "failed.\n";
	}
	delete_file($path);
}

echo "Testing directory:\n";
$path = __DIR__ . '/adir';
$i = 1;
foreach ($iteration as $perms => $exp) {
	create_file($path, $perms);
	clearstatcache(true, $path);
	echo 'Iteration #' . $i++ . ': ';
	if (is_readable($path) == $exp) {
		echo "passed.\n";
	} else {
		var_dump(is_readable($path), $exp);
		echo "failed.\n";
	}
	delete_file($path);
}

?>