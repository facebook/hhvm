<?php


$use_include_path = 


$variation = array(
  'lowercase true' => true,
  'lowercase false' =>false,
  'uppercase TRUE' =>TRUE,
  'uppercase FALSE' =>FALSE,
  );


foreach ( $variation as $var ) {
  var_dump(gzfile( $var ,  $use_include_path ) );
}
?>
===DONE===