<?php

$string = b"\xc3\xa9 uma string utf8 bem formada";

var_dump(preg_match(b'~.*~u', $string, $m, 0, 1));
var_dump($m);
var_dump(preg_last_error() == PREG_BAD_UTF8_OFFSET_ERROR);

var_dump(preg_match(b'~.*~u', $string, $m, 0, 2));
var_dump($m);
var_dump(preg_last_error() == PREG_NO_ERROR);

echo "Done\n";
?>