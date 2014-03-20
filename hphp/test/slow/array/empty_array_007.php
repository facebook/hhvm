<?php

function a() { return array(); }
function main() {
  $x = a();
  $x['foo'][12] = 2;
  return $x;
}
var_dump(main());
