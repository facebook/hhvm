<?php
require_once __DIR__ . '/gmp.inc';

gmp_var_dump(gmp_div_qr());
gmp_var_dump(gmp_div_qr(""));

gmp_var_dump(gmp_div_qr(0, 1));
gmp_var_dump(gmp_div_qr(1, -1));
gmp_var_dump(gmp_div_qr(12653, 23482734));
gmp_var_dump(gmp_div_qr(12653, 23482734, 10));
gmp_var_dump(gmp_div_qr(1123123, 123));
gmp_var_dump(gmp_div_qr(1123123, 123, 1));
gmp_var_dump(gmp_div_qr(1123123, 123, 2));
gmp_var_dump(gmp_div_qr(1123123, 123, GMP_ROUND_ZERO));
gmp_var_dump(gmp_div_qr(1123123, 123, GMP_ROUND_PLUSINF));
gmp_var_dump(gmp_div_qr(1123123, 123, GMP_ROUND_MINUSINF));

$fp = fopen(__FILE__, 'r');

gmp_var_dump(gmp_div_qr($fp, $fp));
gmp_var_dump(gmp_div_qr(array(), array()));
