<?php

function main() {
  $a = array();
  if (isset($GLOBALS['g'])) {
    class bar {}
  } else {
    class bar {}
  }
  for ($i = 0; $i < 2; $i++) {
    if ($i == 1) {
      $a[] = new class extends bar { function foo() { var_dump(__METHOD__); } };
    }

    if ($i == 0) {
      $a[] = new class extends bar { function foo() { var_dump(__METHOD__); } };
    }
  }
  return $a;
}

var_dump(main());
