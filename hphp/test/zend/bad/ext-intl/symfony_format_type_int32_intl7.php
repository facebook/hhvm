<?php


// PHP Unit's code to unserialize data passed as args to #testFormatTypeInt32Intl
$unit_test_args = unserialize('a:4:{i:0;O:15:"NumberFormatter":0:{}i:1;d:-2147483649;i:2;s:19:"SFD2,147,483,647.00";i:3;s:83:"->format() TYPE_INT32 formats inconsistently an integer if out of the 32 bit range.";}');

var_dump($unit_test_args);

// execute the code from #testFormatTypeInt32Intl
$unit_test_args[0]->format($unit_test_args[1], \NumberFormatter::TYPE_INT32);

echo "== didn't crash ==".PHP_EOL;

?>