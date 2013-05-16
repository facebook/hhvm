<?php


echo "*** Test substituting argument 1 with int values ***\n";



$variation_array = array (
    'int 0' => 0,
    'int 1' => 1,
    'int -12345' => -12345,
    );


foreach ( $variation_array as $var ) {
  var_dump(posix_setuid( $var  ) );
}
?>