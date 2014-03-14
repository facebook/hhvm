<?php

$affine_matrix = array(
  'sx' => 1,
  'rx' => 0,
  'ry' => 0,
  'sy' => 1,
  'tx' => 0,
  'ty' => 0
);

$draw = new ImagickDraw;

$draw->affine($affine_matrix);
echo "PASS\n";

unset($affine_matrix['tx']);
try {
  $draw->affine($affine_matrix);
} catch (Exception $ex) {
  echo "PASS\n";
}

try {
  $draw->affine(array());
} catch (Exception $ex) {
  echo "PASS\n";
}
