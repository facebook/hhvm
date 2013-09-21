<?php
$variation_array = array(
  'lowercase true' => true,
  'lowercase false' =>false,
  'uppercase TRUE' =>TRUE,
  'uppercase FALSE' =>FALSE,
  );


foreach ( $variation_array as $var ) {
  var_dump(fstat( $var  ) );
}
?>
===DONE===