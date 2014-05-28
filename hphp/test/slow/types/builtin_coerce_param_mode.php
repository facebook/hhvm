<?php

// HNI ParamCoerceModeNull
var_dump(bcscale([]));
$x = 'bcscale';
var_dump($x([]));

// IDL ParamCoerceModeNull
var_dump(sqrt([]));
$x = 'sqrt';
var_dump($x([]));

// HNI ParamCoerceModeFalse
var_dump(grapheme_extract([], 1));
$x = 'grapheme_extract';
var_dump($x([], 1));
