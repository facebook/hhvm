<?php


$filename = dirname(__FILE__)."/004.txt.gz";


$unset_var = 10;
unset($unset_var);

$variation = array(
  'unset var' => @$unset_var,
  'undefined var' => @$undefined_var,
  'empty string DQ' => "",
  'empty string SQ' => '',
  'uppercase NULL' => NULL,
  'lowercase null' => null,
  );


foreach ( $variation as $var ) {
  var_dump(readgzfile( $filename, $var  ) );
}
?>
===DONE===