<?hh

<<__EntryPoint>>
function main_stream_context_get_default_fopen() {
var_dump(stream_context_get_options(fopen(__FILE__, 'r')));
}
