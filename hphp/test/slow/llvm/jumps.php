<?php


function gotoMain() {
  echo "Entering main\n";
  goto l2;
  l1:
  echo "In l1\n";
  return;
  l2:
  echo "In l2, going to l1\n";
  goto l1;
}

function jccMain($i) {
  if (!$i) {
    echo "i is falsey\n";
  } else {
    echo "i is truthy\n";
  }

  if ($i === 2) {
    echo "i is 2\n";
  } else {
    echo "i is not 2\n";
  }
}

gotoMain();
jccMain(0);
jccMain(2);
