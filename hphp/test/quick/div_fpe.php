<?php

function main() {
  $x = -1;
  $y = -9223372036854775807;
  --$y;
  var_dump($y / $x); // minimum int divided by -1, shouldn't sigfpe

  $x = -1;
  $y = -9223372036854775807;
  --$y;
  var_dump($y % $x); // minimum int mod -1, shouldn't sigfpe
}

main();
