<?php

function bleh() {
  echo "begin\n";
  try {
    echo "try\n";
    yield 1;
    echo "try\n";
    yield 2;
  } finally {
    echo "finally\n";
  }
  echo "end\n";
  yield 3;
}

function main() {
  $xs = bleh();

  foreach ($xs as $x) {
    echo "received: $x\n";
  }
}

main();

