<?hh

<<__EntryPoint>>
function main_stream_get_line_buffer_properly_resized() :mixed{
$fp = fopen(dirname(__FILE__)."/16384chars.txt", 'r');
$init = fgets($fp, 42);
rewind($fp);
$first = stream_get_line($fp, 8191);
$second = stream_get_line($fp, 8192);
echo strlen($second);
}
