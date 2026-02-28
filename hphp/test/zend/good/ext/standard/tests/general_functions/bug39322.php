<?hh <<__EntryPoint>> function main(): void {
$descriptors = dict[
    0 => vec['pipe', 'r'],
    1 => vec['pipe', 'w'],
    2 => vec['pipe', 'w']];

$pipes = vec[];

$process = proc_open('/bin/sleep 120', $descriptors, inout $pipes);

proc_terminate($process, 9);
sleep(1); // wait a bit to let the process finish
var_dump(proc_get_status($process));

echo "Done!\n";
}
