<?php

try {
	$x = create_function('', 'return 1; }');
} catch (ParseError $e) {
	echo "$e\n\n";
}
try {
	$y = create_function('', 'function a() { }; return 2;');
} catch (ParseError $e) {
	echo "$e\n\n";
}
try {
	$z = create_function('', '{');
} catch (ParseError $e) {
	echo "$e\n\n";
}
try {
	$w = create_function('', 'return 3;');
} catch (ParseError $e) {
	echo "$e\n\n";
}

var_dump(
	$y(),
	$w()
);

?>
