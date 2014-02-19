<?php

$im = new Imagick();
$draw = new ImagickDraw();

try {
  $draw->polygon(array(
          array('x' => 1, 'y' => 2),
          array('x' => 'hello', 'y' => array())
          ));

  echo "pass\n";

} catch (Exception $e) {
  echo "fail\n";
}

try {
  $draw->polygon(array(array()));
  echo "fail\n";
} catch (ImagickDrawException $e) {
  echo "pass\n";
}

?>
