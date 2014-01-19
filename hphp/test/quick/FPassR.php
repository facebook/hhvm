<?php

$toSmash = "old";

function smashByVal($param) {
  $param = "newByVal";
}

function smashByRef(&$param) {
  $param = "newByRef";
}
function &retToSmash() {
  global $toSmash;
  return $toSmash;
}

smashByVal(retToSmash());
echo $toSmash . "\n";
smashByRef(retToSmash());
echo $toSmash . "\n";

