<?php
<<__EntryPoint>> function main() {
$a = array();
$a[] = $a;

var_dump($a);
var_dump(json_encode($a));

echo "Done\n";
}
