<?php
function id($x,) {return $x;}
var_dump(id(1,));

function multiline(
  $x,
  $y,
) { return $x+$y; }
var_dump(multiline(
  1,
  2,
));

$x = 3;
$y = 4;
$c = function () use (
  $x,
  $y,
) { return $x+$y; };
var_dump($c());
