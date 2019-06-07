<?hh


<<__EntryPoint>>
function main_bad_json() {
var_dump(json_decode(gzcompress('asdf')));
}
