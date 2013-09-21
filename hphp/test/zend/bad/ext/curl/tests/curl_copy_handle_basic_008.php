<?php
  $host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');

  $url = "{$host}/get.php";
  $ch = curl_init($url);

  curl_setopt($ch, CURLOPT_NOPROGRESS, 0);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_PROGRESSFUNCTION, function() { });
  $ch2 = curl_copy_handle($ch);
  echo curl_exec($ch), PHP_EOL;
  unset($ch);
  echo curl_exec($ch2);

?>