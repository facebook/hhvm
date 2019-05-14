<?php <<__EntryPoint>> function main() {
$a = new SplMaxHeap;
$a->insert($a);
var_dump($a);
echo "===DONE===\n";
}
