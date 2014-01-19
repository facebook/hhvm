<?php

function dec($x) {
  return $x - -(1 << 63);
}

var_dump(dec(1));
