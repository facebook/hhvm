<?php

function a() { return array(); }
function main() {
  $x = a();
  return $x + array(1,2,3);
}
var_dump(main());
