<?hh

<<__EntryPoint>>
function main_fread() :mixed{
$f = fopen('/dev/zero', 'r');
fread($f, 1<<32);
print "Done\n";
}
