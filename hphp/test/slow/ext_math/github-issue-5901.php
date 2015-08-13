<?php

function foo($x, $y)
{
  $x = (int)$x;
  $y = (int)$y;
  min($x, $y);
  echo max($x, $y);
}

foo('30', '40');
