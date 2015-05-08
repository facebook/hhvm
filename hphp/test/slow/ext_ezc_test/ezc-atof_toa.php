<?php
var_dump(ezc_atof_toa("1.25"));
var_dump(ezc_atof_toa("64.25"));
var_dump(ezc_atof_toa("4.09625e+03"));
var_dump(ezc_atof_toa("1234567890123456789.00"));
//
// Unfortunately, zend_strtod doesn't handle Nan's and Infs
//
// var_dump(ezc_atof_toa("NaN"));
// var_dump(ezc_atof_toa("Inf"));
// var_dump(ezc_atof_toa("-Inf"));
var_dump(ezc_atof_toa("-0.0"));
var_dump(ezc_atof_toa("0.0"));
var_dump(ezc_atof_toa(" 1.0"));

echo "Done\n";
?>
