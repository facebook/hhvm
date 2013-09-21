<?php
  $host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');

  $url = "{$host}/get.php?test=";
  $ch  = curl_init();

  curl_setopt($ch, CURLOPT_URL, $url);
  curl_exec($ch);
  $info = curl_getinfo($ch, CURLINFO_EFFECTIVE_URL);
  var_dump($url == $info);
  
  curl_close($ch);
?>
===DONE===