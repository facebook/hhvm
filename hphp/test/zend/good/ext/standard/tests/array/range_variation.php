<?php

echo "\n*** Testing range() with various low and high values ***";
$low_arr = array( "ABCD", -10.5555, TRUE, NULL, FALSE, "", array(1,2));
$high_arr = array( "ABCD", -10.5555, TRUE, NULL, FALSE, "", array(1,2));

for( $i = 0; $i < count($low_arr); $i++) {
  for( $j = 0; $j < count($high_arr); $j++) {
    echo @"\n-- creating an array with low = '$low_arr[$i]' and high = '$high_arr[$j]' --\n";
    var_dump( range( $low_arr[$i], $high_arr[$j] ) );
  }
}

echo "\n*** Possible variatins with steps ***\n";
var_dump( range( 1, 5, TRUE ) );
var_dump( range( 1, 5, array(1, 2) ) );

echo "Done\n";
?>