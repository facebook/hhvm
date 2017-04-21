<?php

function yieldfrom() {
  yield 42;
}

$g = yieldfrom();

foreach($g as $val) { var_dump($val); }
