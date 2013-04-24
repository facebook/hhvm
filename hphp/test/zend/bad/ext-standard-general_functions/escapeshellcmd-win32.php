<?php
echo "*** Testing escapeshellcmd() basic operations ***\n";
$data = array(
	'"abc',
	"'abc",
	'?<>',
	'()[]{}$',
	'%^',
	'#&;`|*?',
	'~<>\\',
	'%NOENV%'
);

$count = 1;
foreach ($data AS $value) {
	echo "-- Test " . $count++ . " --\n";
	var_dump(escapeshellcmd($value));
}

echo "Done\n";
?>