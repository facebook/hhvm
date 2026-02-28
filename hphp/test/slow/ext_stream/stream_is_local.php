<?hh

<<__EntryPoint>>
function main_stream_is_local() :mixed{

var_dump(stream_is_local(4));
var_dump(stream_is_local(""));
var_dump(stream_is_local("\n"));
var_dump(stream_is_local(null));
var_dump(stream_is_local("http://example.com"));

}
