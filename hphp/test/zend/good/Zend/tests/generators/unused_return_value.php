<?php

function gen($foo) { yield; }
<<__EntryPoint>> function main() {
gen('foo'); // return value not used

echo "===DONE===\n";
}
