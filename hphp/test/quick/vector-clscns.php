<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class c {
}

function main($a) {
  return $a[c::BAR];
}
var_dump(main(array('hello there' => 'success')));
