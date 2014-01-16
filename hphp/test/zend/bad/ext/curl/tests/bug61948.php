<?php
  $ch = curl_init();
  var_dump(curl_setopt($ch, CURLOPT_COOKIEFILE, ""));
  var_dump(curl_setopt($ch, CURLOPT_COOKIEFILE, "/tmp/foo"));
  var_dump(curl_setopt($ch, CURLOPT_COOKIEFILE, "/xxx/bar"));
  curl_close($ch);
?>