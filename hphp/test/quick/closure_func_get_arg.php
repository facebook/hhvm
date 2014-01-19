<?php

$a = 1;
$cl = function () use ($a) {
  return func_get_arg(0);
};
var_dump($cl(2));
