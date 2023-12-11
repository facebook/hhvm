<?hh

// Issue #1851.
<<__EntryPoint>>
function main_1851_stream_select() :mixed{
$stream = fopen("php://temp", "r");

$read_streams = vec[$stream];
$write_streams = vec[];
$except = null;

$result = stream_select(
  inout $read_streams,
  inout $write_streams,
  inout $except,
  0,
);

var_dump($write_streams);

fclose($stream);
}
