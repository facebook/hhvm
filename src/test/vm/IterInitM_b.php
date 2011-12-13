<?php

echo "Test begin\n";
function f() {
  $rv = array('a' => 1);
  foreach ($rv as $key => &$splitkeys) {
  }
  return $rv;
}
f();
f();
f();
echo "Test end\n";
