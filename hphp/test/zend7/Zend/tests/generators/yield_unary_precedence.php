<?php
function gen() {
	var_dump(yield +1);
	var_dump(yield -1);
	var_dump(yield * -1); // other ops still should behave normally
}

for ($gen = gen(); $gen->valid(); $gen->send(1)) {
	echo "\n";
	var_dump($gen->current());
}
?>
