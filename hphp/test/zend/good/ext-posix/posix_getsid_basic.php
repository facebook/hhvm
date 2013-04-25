<?php 
  echo "Basic test of posix_getsid function\n"; 
  
  $pid = posix_getpid();	
  $sid = posix_getsid($pid);
  
  var_dump($sid); 
  
?>
===DONE====