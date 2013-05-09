<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

function foo(array $data) {
  $duplicate_click = idx($data, 'duplicate_click');
  if ($duplicate_click === 'false') {
    $duplicate_click = false;
  } else {
    $duplicate_click = (bool)$duplicate_click;
  }
  return $duplicate_click;
}

var_dump(foo(array()));
