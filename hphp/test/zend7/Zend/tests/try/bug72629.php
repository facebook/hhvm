<?php

$var = null;
$e = &$var;

try {
	throw new Exception;
} catch (Exception $e) { }

var_dump($var === $e);

