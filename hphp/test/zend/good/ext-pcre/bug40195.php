<?php

var_dump(preg_match('@^(/([a-z]*))*$@', '//abcde', $m)); var_dump($m);
var_dump(preg_match('@^(/(?:[a-z]*))*$@', '//abcde', $m)); var_dump($m);

var_dump(preg_match('@^(/([a-z]+))+$@', '/a/abcde', $m)); var_dump($m);
var_dump(preg_match('@^(/(?:[a-z]+))+$@', '/a/abcde', $m)); var_dump($m);

?>