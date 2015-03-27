<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function main() {
  $nzero = -1.0;
  while ($nzero != 0.0) {
    $nzero /= 2.0;
  }

  var_dump($nzero);
  var_dump(serialize($nzero));
  var_dump(unserialize(serialize($nzero)));
  print_r($nzero);
}
main();
