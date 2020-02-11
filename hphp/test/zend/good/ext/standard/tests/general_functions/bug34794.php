<?hh <<__EntryPoint>> function main(): void {
echo "Opening process 1\n";
$pipes1 = null;
$process1 = proc_open('/bin/cat', darray[0 => varray['pipe', 'r'], 1 =>varray['pipe', 'r']], inout $pipes1);

echo "Opening process 2\n";
$pipes2 = null;
$process2 = proc_open('/bin/cat', darray[0 => varray['pipe', 'r'], 1 =>varray['pipe', 'r']], inout $pipes2);


echo "Closing process 1\n";
fclose($pipes1[0]);
fclose($pipes1[1]);
proc_close($process1);

echo "Closing process 2\n";
fclose($pipes2[0]);
fclose($pipes2[1]);
proc_close($process2);

echo "Done\n";
}
