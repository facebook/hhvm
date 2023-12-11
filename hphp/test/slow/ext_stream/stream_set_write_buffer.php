<?hh


<<__EntryPoint>>
function main_stream_set_write_buffer() :mixed{
$descriptorspec = dict[
  0 => vec["pipe", "r+"],
  1 => vec["pipe", "w"],
  2 => vec["pipe", "a"],
];

$io = null;
$process = proc_open('echo', $descriptorspec, inout $io);
var_dump(stream_set_write_buffer($io[0], 0));


$fd = fopen(__DIR__.'/stream_set_write_buffer.php.sample', 'rb');
var_dump(stream_set_write_buffer($fd, 0));
var_dump(stream_set_write_buffer($fd, 4096));
var_dump(trim(fgets($fd)));
fclose($fd);
}
