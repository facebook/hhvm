<?php 
  echo "Basic test of posix_getpgid function\n"; 
  
  $pid = posix_getpid();	
  $pgid = posix_getpgid($pid);
  
  var_dump($pgid); 
  
?>
===DONE====