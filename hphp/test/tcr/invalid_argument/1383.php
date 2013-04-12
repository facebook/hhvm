<?php

 function handler($err, $errstr) {  $errstr = preg_replace('/given,.*$/','given', $errstr);  var_dump($err, $errstr);}set_error_handler('handler');class y {}class x {  function __construct(y $y) {}}var_dump(new X(null));