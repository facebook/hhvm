<?php

var_dump(strlen(random_bytes(7)));
var_dump(random_int(1, 10));

// Astronomically low chance of a false positive here, and making sure we don't
// accidentally return a constant value is worth it.
var_dump(random_bytes(16) === random_bytes(16));
var_dump(
  random_int(PHP_INT_MIN, PHP_INT_MAX) === random_int(PHP_INT_MIN, PHP_INT_MAX)
);
