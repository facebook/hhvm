<?hh


<<__EntryPoint>>
function main_directory_stream_metadata() :mixed{
$stream = opendir('.');
var_dump(stream_get_meta_data($stream));
}
