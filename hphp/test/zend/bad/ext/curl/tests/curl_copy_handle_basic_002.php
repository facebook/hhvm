<?php
  $host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');

  echo '*** Testing curl copy handle with simple POST ***' . "\n";

  $url = "{$host}/get.php?test=getpost";
  $ch = curl_init();

  ob_start(); // start output buffering
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_setopt($ch, CURLOPT_POSTFIELDS, "Hello=World&Foo=Bar&Person=John%20Doe");
  curl_setopt($ch, CURLOPT_URL, $url); //set the url we want to use
  
  $copy = curl_copy_handle($ch);
  curl_close($ch);
 
  $curl_content = curl_exec($copy);
  curl_close($copy);

  var_dump( $curl_content );
?>
===DONE===