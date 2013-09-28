<?php

function callback($data) {
  return "callback: $data";
}
ob_start();
echo "from first level\n";
ob_start();
ob_start("callback");
echo "foobar!\n";
exit;
