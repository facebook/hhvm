<?php

// Taken from pow-operator PHP RFC (see https://wiki.php.net/rfc/pow-operator)
var_dump(2 ** 3 ** 2); // 512 (not 64)
var_dump(-3 ** 2); // -9 (not 9)
var_dump(1 - 3 ** 2); // -8
var_dump(~3 ** 2); // -10 (not 16)
