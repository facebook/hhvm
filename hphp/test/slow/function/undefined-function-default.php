<?php

function f($val = g()) {
  echo "val = ";
  var_dump($val);
}

f();
