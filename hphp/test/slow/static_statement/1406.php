<?php

function test() {
  static $commenced = false;
  if ($commenced === false) {
    return 1;
  }
  $commenced = true;
  unset($args);
}
echo test();
