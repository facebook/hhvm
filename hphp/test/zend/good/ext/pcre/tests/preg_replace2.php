<?hh
<<__EntryPoint>> function main(): void {
var_dump(preg_replace('', varray[], ''));

var_dump(preg_replace(varray['/\da(.)/ui', '@..@'], '$1', '12Abc'));
var_dump(preg_replace(varray['/\da(.)/ui', '@(.)@'], '$1', varray['x','a2aA', '1av2Ab']));


var_dump(preg_replace(varray['/[\w]+/'], varray['$'], varray['xyz', 'bdbd']));
var_dump(preg_replace(varray['/\s+/', '~[b-d]~'], varray['$'], varray['x y', 'bd bc']));

echo "==done==\n";
}
