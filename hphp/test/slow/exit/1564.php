<?hh


<<__EntryPoint>>
function main_1564() {
$command = "exit 2";
$return = -1;
system($command, inout $return);
print "$return\n";
}
