<?php

var_dump(preg_match('', '', $match, 0xfff));

var_dump(preg_match('/\d+/', '123 456 789 012', $match, 0, -8));
var_dump($match);

var_dump(preg_match('/\d+/', '123 456 789 012', $match, 0, -500));
var_dump($match);

var_dump(preg_match_all('/\d+/', '123 456 789 012', $match, 0, -8));
var_dump($match);

var_dump(preg_match('/(?P<3>)/', ''));

?>