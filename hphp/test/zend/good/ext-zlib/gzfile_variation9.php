<?php


$filename = dirname(__FILE__)."/004.txt.gz";

$variation = array(
  'lowercase true' => true,
  'lowercase false' =>false,
  'uppercase TRUE' =>TRUE,
  'uppercase FALSE' =>FALSE,
  );


foreach ( $variation as $var ) {
  var_dump(gzfile( $filename, $var  ) );
}
?>
===DONE===