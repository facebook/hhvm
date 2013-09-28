<?php

$array = new SplFixedArray( 4 );

$array[0] = "Hello";
$array[1] = "world";
$array[2] = "elePHPant";

$array->rewind( "invalid" );

?>