<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class C {}

function elem() {
  $k = new C();

  $b = array(4, 3);
  @var_dump($b[$k] = 4);

  $b = array(4, 3);
  @var_dump($b[$k] += 4);

  $b = array(4, 3);
  @var_dump($b[$k]++);
}


<<__EntryPoint>>
function main_minstr_weird_key() {
elem();
}
