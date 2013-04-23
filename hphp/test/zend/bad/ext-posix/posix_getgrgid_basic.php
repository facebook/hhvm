<?php 
  echo "Basic test of POSIX getgid and getgrid fucntions\n"; 
  	
  $gid = posix_getgid();
  $groupinfo = posix_getgrgid($gid);
  
  print_r($groupinfo);
  
?>
===DONE===