<?php

function main() {
  $a = hphp_miarray();
  for ($i = 0; $i < 10000; $i++) {
    $a[$i] = $i + 1;
  }
  $a["foo"] = 10;

  $b = build();
  $b["foo"] = 10;
}

function build() {
  $a = hphp_miarray();
  for ($i = 0; $i < 10000; $i++) {
    $a[$i] = $i + 1;
  }
  return $a;
}

main();
