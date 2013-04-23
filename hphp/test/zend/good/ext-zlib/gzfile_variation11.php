<?php


$filename = dirname(__FILE__)."/004.txt.gz";


$variation = array(
  'float 10.5' => 10.5,
  'float -10.5' => -10.5,
  'float 12.3456789000e10' => 12.3456789000e10,
  'float -12.3456789000e10' => -12.3456789000e10,
  'float .5' => .5,
  );


foreach ( $variation as $var ) {
  var_dump(gzfile( $filename, $var  ) );
}
?>
===DONE===