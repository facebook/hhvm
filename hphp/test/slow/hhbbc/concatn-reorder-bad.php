<?php

function f(\HH\string $a, \HH\string $b) {
  return "[".$a.$b."]\n";
}

echo f('a', 'b');
