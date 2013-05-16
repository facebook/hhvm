<?php


echo "*** Test substituting argument 1 with boolean values ***\n";



$variation_array = array(
  'lowercase true' => true,
  'lowercase false' =>false,
  'uppercase TRUE' =>TRUE,
  'uppercase FALSE' =>FALSE,
  );


foreach ( $variation_array as $var ) {
  var_dump(posix_setuid( $var  ) );
}
?>