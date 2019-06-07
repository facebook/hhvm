<?hh


<<__EntryPoint>>
function main_more_ini() {
var_dump(parse_ini_string(
"a=1
a[]=1
"));

var_dump(parse_ini_string(
"foo[] = 'bar'
foo[1] = 'baz'
foo[] = 'bah'
"));
}
