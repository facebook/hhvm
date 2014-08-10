<?php
$s_string = '1111111111';
$s_search = '/1/';
$s_replace = 'One ';
$i_limit = 1;
$i_count = 0;

$s_output = preg_replace($s_search, $s_replace, $s_string, $i_limit,
$i_count);
echo "Output = " . var_export($s_output, True) . "\n";
echo "Count  = $i_count\n";
var_dump(preg_last_error() === PREG_NO_ERROR);

$i_limit = strlen($s_string);
$s_output = preg_replace($s_search, $s_replace, $s_string, $i_limit,
$i_count);
echo "Output = " . var_export($s_output, True) . "\n";
echo "Count  = $i_count\n";
var_dump(preg_last_error() === PREG_NO_ERROR);

?>
