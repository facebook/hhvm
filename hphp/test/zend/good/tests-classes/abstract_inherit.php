<?php

abstract class pass {
	abstract function show();
}

abstract class fail extends pass {
}

$t = new fail();
$t = new pass();

echo "Done\n"; // Shouldn't be displayed
?>