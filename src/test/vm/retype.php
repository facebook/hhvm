<?php

function show($x) {
  echo $x;

  if(true) {
    echo "t\n";
  } else {
    echo "f\n";
  }
}

show(7);
show('test');


