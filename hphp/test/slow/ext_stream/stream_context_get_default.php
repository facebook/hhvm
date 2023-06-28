<?hh

<<__EntryPoint>>
function main_stream_context_get_default() :mixed{
var_dump(stream_context_get_options(stream_context_get_default()));
var_dump(stream_context_get_options(stream_context_get_default(
  darray["http" => darray["header" => "X-Hello: world"]])));
var_dump(stream_context_get_options(stream_context_get_default()));
var_dump(stream_context_get_options(stream_context_set_default(
  darray["http" => darray["method" => "POST"]])));
var_dump(stream_context_get_options(stream_context_get_default()));
}
