<?php 
  $data = @$not_exists; 
  $data = @($not_exists); 
  $data = @!$not_exists; 
  $data = !@$not_exists; 
  $data = @($not_exists+1); 
  echo "ok\n";
?>