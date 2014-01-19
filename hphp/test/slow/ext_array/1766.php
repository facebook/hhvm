<?php

error_reporting(0);
var_dump(range('', '', 1));
var_dump(range('', '', -1));
var_dump(range('9', '10', -1));
var_dump(range(9, 10, -1));
var_dump(range(9, 10, -1.5));
var_dump(range(9, 10, 33333333.33));
var_dump(range(9, 10, -33333333.33));
var_dump(range(9223372036854775807, 9223372036854775805, -1));
var_dump(range(9223372036854775807, 9223372036854775805,               9223372036854775807));
var_dump(range(9223372036854775807, 9223372036854775805,               -9223372036854775807));
var_dump(range(9223372036854775807, 9223372036854775805,               2147483648));
var_dump(range(9223372036854775807, 9223372036854775805,               -2147483648));
var_dump(range('9', '10', '-1'));
var_dump(range('9', '10', '-1.5'));
var_dump(range('9', '10', '33333333.33'));
var_dump(range('9', '10', '-33333333.33'));
var_dump(range('9223372036854775807', '9223372036854775805', '-1'));
var_dump(range('9223372036854775807', '9223372036854775805',               '9223372036854775807'));
var_dump(range('9223372036854775807', '9223372036854775805',               '-9223372036854775807'));
var_dump(range(null, null, -2.5));
var_dump(range(null, null, 3.5));
var_dump(range(null, null, null));
var_dump(range(3.5, -4.5, null));
