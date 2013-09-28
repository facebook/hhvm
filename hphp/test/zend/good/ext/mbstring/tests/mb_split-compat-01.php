<?php
/* (counterpart: ext/standard/tests/reg/009.phpt) */
	$a=mb_split("[[:space:]]","this is	a
test");
	echo count($a) . "\n";
	for ($i = 0; $i < count($a); $i++) {
          echo $a[$i] . "\n";
        }
?>