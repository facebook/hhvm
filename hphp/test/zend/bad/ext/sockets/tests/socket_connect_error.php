<?php
  // Test with no arguments
  $server = socket_create();
  
  // Test with less arguments than required
  $server = socket_create(SOCK_STREAM, getprotobyname('tcp'));
  
  // Test with non integer parameters
  $server = socket_create(array(), 1, 1);
  
?>