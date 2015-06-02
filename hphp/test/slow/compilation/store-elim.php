<?php

function main($a, $b, $c) {
  $i = 0;
  if ($c) {
    $t = 0;
    goto head;
  } else {
    $t = 1;
    goto head;
  }
  if ($a > 0) {
    head:
    if ($i & 1) {
      $t = 2;
    }
    if (++$i < $a) goto head;
  }

  if ($b == 42) {
    $t = 4;
  }

  var_dump($t);
}

for ($i = 0; $i < 2; $i++) {
  main(1, 42, 0);
  main(2, 42, 0);
  main(1, 43, 0);
  main(2, 43, 0);

  main(1, 42, 1);
  main(2, 42, 1);
  main(1, 43, 1);
  main(2, 43, 1);
}
