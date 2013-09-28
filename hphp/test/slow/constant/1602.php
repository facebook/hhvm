<?php

var_dump(define('KONST', array('a', 'bc')));
var_dump(KONST);
var_dump(define('FLUB', 1230));
var_dump(define('FLUB', array(1,23)));
var_dump(FLUB);
var_dump(define('BLAH', array_map('strlen', array('a', 'bc'))));
var_dump(BLAH);
define('FOO', array(1,2,3));
