<?php
  $ch = curl_init();
  var_dump(curl_setopt($ch, CURLOPT_COOKIEFILE, ""));
  var_dump(curl_setopt($ch, CURLOPT_COOKIEFILE, "c:/tmp/foo"));
  var_dump(curl_setopt($ch, CURLOPT_COOKIEFILE, "c:/xxx/bar"));
  curl_close($ch);
?>