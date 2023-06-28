<?hh


<<__EntryPoint>>
function main_stream_set_read_buffer() :mixed{
$descriptorspec = darray[
  0 => varray["pipe", "r+"],
  1 => varray["pipe", "w"],
  2 => varray["pipe", "a"],
];

$io = null;
$process = proc_open('echo', $descriptorspec, inout $io);
var_dump(stream_set_read_buffer($io[0], 0));


$fd = fopen(__DIR__.'/stream_set_read_buffer.php.sample', 'rb');
var_dump(stream_set_read_buffer($fd, 0));
var_dump(stream_set_read_buffer($fd, 4096));
var_dump(trim(fgets($fd)));
fclose($fd);
}
