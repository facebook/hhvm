<?hh <<__EntryPoint>> function main(): void {
$descriptors = darray[
    0 => varray['pipe', 'r'],
    1 => varray['pipe', 'w'],
    2 => varray['pipe', 'w']];

$pipes = varray[];

$process = proc_open('/bin/sleep 120', $descriptors, inout $pipes);

proc_terminate($process, 9);
sleep(1); // wait a bit to let the process finish
var_dump(proc_get_status($process));

echo "Done!\n";
}
