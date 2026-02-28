<?hh


<<__EntryPoint>>
function main_bad_json() :mixed{
var_dump(json_decode(gzcompress('asdf')));
}
