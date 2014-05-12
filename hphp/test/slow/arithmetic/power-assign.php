<?php

function main() {
  $io = 1;
  $do = 1.5;

  $it = 2;
  $dt = 2.3;

  $ix = 10;
  $dx = 10.7;

  var_dump($io **= $ix);
  var_dump($dx **= $dx);

  var_dump($it **= $dt);
  var_dump($dt **= $it);
}

main();
