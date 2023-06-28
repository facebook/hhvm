<?hh


<<__EntryPoint>>
function main_stream_set_chunk_size() :mixed{
$fd = fopen(__DIR__.'/stream_set_chunk_size.php.sample', 'rb');
var_dump(stream_set_chunk_size($fd, 256));
var_dump(stream_set_chunk_size($fd, 8192));
var_dump(stream_set_chunk_size($fd, 0));
fclose($fd);
}
