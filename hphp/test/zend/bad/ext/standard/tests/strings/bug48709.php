<?php

/* Initial letter exceptions */
$exceptions = array(
	'kn', // Drop first letter
	'gn', // ditto
	'pn', // ditto
	'ae', // ditto
	'wr', // ditto
	'x',  // s
	'wh', // w
	'wa'  // w
);	
	
foreach ($exceptions as $letter) {
	printf("%s => %s\n", $letter, metaphone($letter));
}

?>