<?php


$filename = dirname(__FILE__)."/004.txt.gz";
$use_include_path = false;
$extra_arg = 'nothing'; 

var_dump(gzfile( $filename, $use_include_path, $extra_arg ) );

var_dump(gzfile(  ) );


?>
===DONE===