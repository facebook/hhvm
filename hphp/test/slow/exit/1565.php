<?php


<<__EntryPoint>>
function main_1565() {
$command = "exit 2";
passthru($command, &$return);
print "$return\n";
}
