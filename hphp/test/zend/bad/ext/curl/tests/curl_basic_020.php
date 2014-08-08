<?php
  include 'server.inc';
  $host = curl_cli_server_start();

  $url = "{$host}/get.php?test=";
  $ch  = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_exec($ch);
  var_dump(curl_getinfo($ch, CURLINFO_HTTP_CODE));
  curl_close($ch);
?>
===DONE===
