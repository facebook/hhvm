<?php

$array = new SplFixedArray( 3 );

$array[0] = "Hello";
$array[1] = "world";
$array[2] = "elePHPant";

foreach ( $array as $value ) {
	echo $array->current( array("this","should","not","execute") );
}

?>