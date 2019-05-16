<?php
<<__EntryPoint>> function main() {
$str = "a b c d e";
var_dump(count(sscanf("a ",'%1$s')));

echo "Done\n";
}
