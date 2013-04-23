<?php

var_dump(preg_match_all('/^\S+.+$/', "aeiou\n", $m));
var_dump($m);

var_dump(preg_match_all('/^\S+.+$/D', "aeiou\n", $m));
var_dump($m);

var_dump(preg_match_all('/^\S+\s$/D', "aeiou\n", $m));
var_dump($m);

?>