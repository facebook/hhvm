<?php
function gen(array $a) { yield; }
try {
	gen(42);
} catch (TypeError $e) {
	echo $e->getMessage()."\n";
}

try {
	foreach (gen(42) as $val) {
		var_dump($val);
	}
} catch (TypeError $e) {
        echo $e->getMessage()."\n";
}
?>
