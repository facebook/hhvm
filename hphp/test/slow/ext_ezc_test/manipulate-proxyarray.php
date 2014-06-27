<?php

// Test of ProxyArray::Copy()

$x = array_merge([], ezc_create_cloneable_in_array());
$x += [];
var_dump( $x );
$x = null;
print "End\n";
