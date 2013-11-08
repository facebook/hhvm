<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function main($a, $i) {
  var_dump(isset($a[1 << 32]));
  var_dump(isset($a[$i]));
}

main(array(1, 2, 3, 4), 1 << 32 + 1);
