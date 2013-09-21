<?php


$filename = dirname(__FILE__)."/004.txt.gz";


$variation = array (
    'int 0' => 0,
    'int 1' => 1,
    'int 12345' => 12345,
    'int -12345' => -2345,
    );


foreach ( $variation as $var ) {
  var_dump(readgzfile( $filename, $var  ) );
}
?>
===DONE===