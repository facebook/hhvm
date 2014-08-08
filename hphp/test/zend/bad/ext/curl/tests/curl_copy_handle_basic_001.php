<?php

  include 'server.inc';
  $host = curl_cli_server_start();

  echo '*** Testing curl copy handle with simple GET ***' . "\n";

  $url = "{$host}/get.php?test=getpost&get_param=Hello%20World";
  $ch = curl_init();

  ob_start(); // start output buffering
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_URL, $url); //set the url we want to use
  
  $copy = curl_copy_handle($ch);
  curl_close($ch);
  
  $curl_content = curl_exec($copy);
  curl_close($copy);

  var_dump( $curl_content );
?>
===DONE===
