<?php
ini_set('intl.error_level', E_WARNING);
$x = new IntlDateFormatter(1,1,1,1,1);
var_dump($x->__construct(1,1,1,1,1));