<?php

function a() { return array(); }
function main() {
  $x = a();
  $x[] = 2;
  return $x;
}

var_dump(main());
