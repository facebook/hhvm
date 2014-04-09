<?php

function main() {
  $ch = curl_init();
  var_dump(is_resource($ch));
  curl_close($ch);
  var_dump(is_resource($ch));

  $ch = curl_multi_init();
  var_dump(is_resource($ch));
  curl_multi_close($ch);
  var_dump(is_resource($ch));
}

main();
