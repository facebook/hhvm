<?hh


<<__EntryPoint>>
function main_proc_open_empty_pipes() :mixed{
$command = 'echo "foo" >> /dev/null';
$descriptors = varray[
  varray['pipe', 'r'],
  varray['pipe', 'w'],
];

$pipes = varray[1, 2, 3, 4, 5];
$proc = proc_open($command, darray($descriptors), inout $pipes);
var_dump($proc);
var_dump($pipes);
}
