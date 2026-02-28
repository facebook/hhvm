<?hh


<<__EntryPoint>>
function main_1564() :mixed{
$command = "exit 2";
$return = -1;
system($command, inout $return);
print "$return\n";
}
