<?hh

<<__EntryPoint>>
function main_bad_open() {
$json = '{"foo": { "package": { "bar": "b{az" }}}}';
var_dump(json_decode($json));
}
