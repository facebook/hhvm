<?hh


<<__EntryPoint>>
function main_1565() {
$command = "exit 2";
$return = -1;
passthru($command, inout $return);
print "$return\n";
}
