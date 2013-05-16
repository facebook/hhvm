<?php
  $url = 'http://www.example.com/'; 
  $ch  = curl_init($url);
  var_dump($url == curl_getinfo($ch, CURLINFO_EFFECTIVE_URL));
?>
===DONE===