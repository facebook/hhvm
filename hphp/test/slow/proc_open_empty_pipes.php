<?hh


<<__EntryPoint>>
function main_proc_open_empty_pipes() :mixed{
$command = 'echo "foo" >> /dev/null';
$descriptors = vec[
  vec['pipe', 'r'],
  vec['pipe', 'w'],
];

$pipes = vec[1, 2, 3, 4, 5];
$proc = proc_open($command, darray($descriptors), inout $pipes);
var_dump($proc);
var_dump($pipes);
}
