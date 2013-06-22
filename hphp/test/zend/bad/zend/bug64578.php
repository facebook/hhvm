<?php

set_error_handler(function($no, $err) { var_dump($err); });

function x($s) { $s['a'] = 1; };
$y = '1';
x($y);
print_r($y);