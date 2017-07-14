<?php
for ($i = 0; $i < 3; $i++) {
	foreach ([1,2,3] as &$val) {
		echo "$val\n";
	}
}
?>
