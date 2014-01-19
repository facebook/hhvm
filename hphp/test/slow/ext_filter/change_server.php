<?php

var_dump($_SERVER['PHP_SELF']);
$_SERVER['PHP_SELF'] = 'foo';
var_dump($_SERVER['PHP_SELF']);
var_dump(filter_input(INPUT_SERVER, 'PHP_SELF'));
