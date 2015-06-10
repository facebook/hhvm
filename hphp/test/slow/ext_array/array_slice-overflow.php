<?php

const OFFSET = 0x7FFFFFFFFFFFFFFF;

function make_array() {
  return array_slice(
    array(
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    ),
    OFFSET,
    OFFSET,
    true
  );
}

var_dump(make_array());
