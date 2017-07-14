<?php

for ($a = 1, $b = 1; $a < 100; $b = $a + $b, $a = $b - $a) {
  echo $a . "\n";
}
