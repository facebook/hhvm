<?php
<<__EntryPoint>> function main() {
$str = "a b c d e";
var_dump(sscanf("a ",'%1$s',&$str));

echo "Done\n";
}
