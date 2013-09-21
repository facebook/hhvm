<?php
$variation_array = array(
  'float 10.5' => 10.5,
  'float -10.5' => -10.5,
  'float 12.3456789000e10' => 12.3456789000e10,
  'float -12.3456789000e10' => -12.3456789000e10,
  'float .5' => .5,
  );


foreach ( $variation_array as $var ) {
  var_dump(fstat( $var  ) );
}
?>
===DONE===