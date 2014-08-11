<?php

// These conversions ignore the provided base and assume it's hex given the
// input starts with '0x'. This matches PHP5 behavior.

function gmp_noop_convert($str, $base) {
  var_dump(gmp_strval(gmp_init($str, $base), $base));
}

gmp_noop_convert('0x123', 16);
gmp_noop_convert('0x123', 34);
gmp_noop_convert('0x123', 35);
gmp_noop_convert('0x123', 36);

gmp_noop_convert('0xggg', 34);
gmp_noop_convert('0xggg', 35);
gmp_noop_convert('0xggg', 36);
