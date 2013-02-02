<?php

function foo($x, $y) {
  var_dump($x == $y);
  var_dump($x === $y);
}

$x1 = new stdClass;
$x2 = new stdClass;
$y1 = curl_init();
$y2 = curl_init();

foo($x1, $x1);
foo($x1, $x2);
foo($x1, $y1);
foo($x1, $y2);

foo($x2, $x1);
foo($x2, $x2);
foo($x2, $y1);
foo($x2, $y2);

foo($y1, $x1);
foo($y1, $x2);
foo($y1, $y1);
foo($y1, $y2);

foo($y2, $x1);
foo($y2, $x2);
foo($y2, $y1);
foo($y2, $y2);

