<?php
<<__EntryPoint>> function main() {
$dir = 'ftp://your:self@localhost/';

var_dump(@opendir($dir));
var_dump(@opendir($dir));

echo "===DONE===\n";
}
