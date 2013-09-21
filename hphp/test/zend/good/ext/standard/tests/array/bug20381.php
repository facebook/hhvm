<?php
$a = array(
	'a1' => 1,
	'a2' => array( 1, 2, 3 ),
	'a3' => array(
		'a' => array( 10, 20, 30 ),
		'b' => 'b'
		)
	);
$b = array( 'a1' => 2,
	'a2' => array( 3, 4, 5 ),
	'a3' => array(
		'c' => 'cc',
		'a' => array( 10, 40 )
		)
	);

var_dump($a);
array_merge_recursive( $a, $b );
var_dump($a);
?>