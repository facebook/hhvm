<?php

function main() {
  $port = $_ENV['SERVERPORT'];
  $host = php_uname('n');
  $url = "http://$host:$port/hello.php";

  $ch1 = curl_init();
  $ch2 = curl_init();

  curl_setopt($ch1, CURLOPT_URL, $url);
  curl_setopt($ch1, CURLOPT_HEADER, 0);
  curl_setopt($ch1, CURLOPT_WRITEFUNCTION, 'except');
  curl_setopt($ch2, CURLOPT_URL, $url);
  curl_setopt($ch2, CURLOPT_HEADER, 0);

  $mh = curl_multi_init();

  curl_multi_add_handle($mh,$ch1);
  curl_multi_add_handle($mh,$ch2);

  $active = null;
  do {
    $mrc = curl_multi_exec($mh, $active);
  } while ($mrc == CURLM_CALL_MULTI_PERFORM);

  $ret = "";
  while ($active && $mrc == CURLM_OK) {
    if (curl_multi_select($mh) != -1) {
      do {
        try {
          $mrc = curl_multi_exec($mh, $active);
        } catch (Exception $e) {
          $ret .= ":::Exception: " . $e->getMessage() . "\n";
        }
      } while ($mrc == CURLM_CALL_MULTI_PERFORM);
    }
  }

  curl_multi_close($mh);
  return $ret;
}

function except() {
  throw new Exception("oops");
}

echo main();
