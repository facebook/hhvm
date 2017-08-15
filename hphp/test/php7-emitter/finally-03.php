<?php

$x = 0;
$success = true;
while ($x++ < 5) {
  try {
    break;
    $success = false;
  }
  finally {
    var_dump("yes");
  }
}

var_dump($success);

$x = 0;
while ($x++ < 5) {
  try {
    continue;
  }
  finally {
    var_dump($x);
  }
}
