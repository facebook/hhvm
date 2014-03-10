<?php

$value = 750000.0;

// Null should fallback to default separators.
var_dump(number_format($value, 2, null, ''));
var_dump(number_format($value, 2, '', null));
