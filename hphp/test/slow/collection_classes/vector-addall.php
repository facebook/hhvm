<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function main($v) {
  for ($i = 0; $i < 10; $i++) $v->add($i);
  return $v->addAll($v);
}

var_dump(main(Vector { 1, 2, 3 }));
