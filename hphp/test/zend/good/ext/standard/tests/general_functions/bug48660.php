<?php

$ini_location = dirname(__FILE__) . '/bug48660.tmp';

// Build ini data
$ini_data = '
[cases]

Case.a = avalue
Case.b = "$dollar_sign"
Case.c = "dollar_sign$"
Case.d = "$dollar_sign$"
Case.e = 10
';

// Save ini data to file
file_put_contents($ini_location, $ini_data);

var_dump(parse_ini_file($ini_location, true, INI_SCANNER_RAW));
var_dump(parse_ini_file($ini_location, true, INI_SCANNER_NORMAL));

?>
<?php @unlink(dirname(__FILE__) . '/bug48660.tmp'); ?>