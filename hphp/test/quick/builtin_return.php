<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(array $data) {
  $ret = current($data);
  if ($ret === 'false') {
    $ret = false;
  } else {
    $ret = (bool)$ret;
  }
  return $ret;
}

var_dump(foo(array()));
