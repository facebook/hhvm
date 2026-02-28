<?hh
<<__EntryPoint>> function main(): void {
var_dump(preg_replace('', vec[], ''));

var_dump(preg_replace(vec['/\da(.)/ui', '@..@'], '$1', '12Abc'));
var_dump(preg_replace(vec['/\da(.)/ui', '@(.)@'], '$1', vec['x','a2aA', '1av2Ab']));


var_dump(preg_replace(vec['/[\w]+/'], vec['$'], vec['xyz', 'bdbd']));
var_dump(preg_replace(vec['/\s+/', '~[b-d]~'], vec['$'], vec['x y', 'bd bc']));

echo "==done==\n";
}
