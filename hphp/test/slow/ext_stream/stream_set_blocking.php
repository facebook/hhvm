<?hh


<<__EntryPoint>>
function main_stream_set_blocking() :mixed{
$fd = fopen(__DIR__.'/stream_set_blocking.php.sample', 'rb');
var_dump(stream_set_blocking($fd, true));
var_dump(stream_set_blocking($fd, false));
var_dump(trim(fgets($fd)));
fclose($fd);
}
