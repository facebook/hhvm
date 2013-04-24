<?php

$a1 = array( 'key1' => 1, 'key3' => 2 );
$a2 = array();
$a1 = array_merge_recursive( $a1, $a2 );
$a1 = array_merge_recursive( $a1, $a2 );
unset( $a1, $a2 );

$a1 = array();
$a2 = array( 'key1' => 1, 'key3' => 2 );
$a1 = array_merge_recursive( $a1, $a2 );
$a1 = array_merge_recursive( $a1, $a2 );
unset( $a1, $a2 );

$a1 = array();
$a2 = array( 'key1' => &$a1 );
$a1 = array_merge_recursive( $a1, $a2 );
$a1 = array_merge_recursive( $a1, $a2 );
unset( $a1, $a2 );

$x = 'foo';
$y =& $x;
$a1 = array($x, $y, $x, $y);
$a2 = array( 'key1' => $a1, $x, $y );
$a1 = array_merge_recursive( $a1, $a2 );
$a1 = array_merge_recursive( $a1, $a2 );
unset( $a1, $a2 );

?>