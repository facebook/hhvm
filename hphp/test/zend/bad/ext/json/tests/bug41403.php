<?php

setlocale(LC_NUMERIC, 'de_DE');
var_dump(json_decode('[2.1]'));
var_dump(json_decode('[0.15]'));
var_dump(json_decode('[123.13452345]'));
var_dump(json_decode('[123,13452345]'));

echo "Done\n";
?>