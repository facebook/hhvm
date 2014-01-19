<?php
  $host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');

  $url = "{$host}/get.php?test=";
  $ch  = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_exec($ch);
  var_dump(curl_getinfo($ch, CURLINFO_HTTP_CODE));
  curl_close($ch);
?>
===DONE===