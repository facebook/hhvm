<?php
ini_set('intl.error_level', E_WARNING);
$x = new IntlDateFormatter('en', 1, 1);
var_dump($x->__construct('en', 1, 1));