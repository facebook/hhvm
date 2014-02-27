<?php

function main() {
  var_dump(
    curl_setopt(
      curl_init(),
      CURLOPT_STDERR,
      fopen('php://temp', 'r+')
    )
  );
}

main();
