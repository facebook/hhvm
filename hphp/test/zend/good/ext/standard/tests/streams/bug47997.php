<?hh
<<__EntryPoint>> function main(): void {
$in = fopen('data://text/plain,', 'rb+');
$out = fopen('php://memory', 'wb+');

var_dump(stream_copy_to_stream($in, $out));
}
