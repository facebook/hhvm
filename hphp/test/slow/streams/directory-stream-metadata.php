<?hh


<<__EntryPoint>>
function main_directory_stream_metadata() {
$stream = opendir('.');
var_dump(stream_get_meta_data($stream));
}
