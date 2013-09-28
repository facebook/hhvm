<?php

var_dump(preg_match('/\PN+/', '123abc', $m));
var_dump($m);

var_dump(preg_match('/\P{N}+/A', '123abc'));
var_dump(preg_match('/^\P{N}+/', '123abc'));
var_dump(preg_match('/^\P{N}+/A', '123abc'));

?>