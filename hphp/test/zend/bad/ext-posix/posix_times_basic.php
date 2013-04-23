<?php 
  echo "Basic test of POSIX times function\n"; 
  	
  $times = posix_times();
  
  var_dump($times); 
  
  
  if ($times == FALSE) {
  	$errno= posix_get_last_error();
  	var_dump(posix_strerror($errno)); 
  }
  
?>
===DONE====