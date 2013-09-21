<?php 
  echo "Basic test of POSIX getpgrp function\n"; 
  	
  $pgrp = posix_getpgrp();
  
  var_dump($pgrp); 
  
?>
===DONE====