<?php
  $ch   = curl_init();
  $info = curl_getinfo($ch);
  var_dump($info);
?>
===DONE===