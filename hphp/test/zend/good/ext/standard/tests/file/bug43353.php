<?php

var_dump(is_dir('file:///datafoo:test'));
var_dump(is_dir('datafoo:test'));
var_dump(file_get_contents('data:text/plain,foo'));
var_dump(file_get_contents('datafoo:text/plain,foo'));

?>
