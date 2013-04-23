<?php

var_dump(preg_match_all('/(.)x/', 'zxax', $match, PREG_PATTERN_ORDER));
var_dump($match);

var_dump(preg_match_all('/(.)x/', 'zxyx', $match, PREG_SET_ORDER));
var_dump($match);

var_dump(preg_match_all('/(.)x/', 'zxyx', $match, PREG_OFFSET_CAPTURE));
var_dump($match);

var_dump(preg_match_all('/(.)x/', 'zxyx', $match, PREG_SET_ORDER | PREG_OFFSET_CAPTURE));
var_dump($match);

?>