<?hh


<<__EntryPoint>>
function main_var_export_arrays() {
echo("** array **\n");
var_export(array());
echo("\n\n");
var_export(array('foo'));
echo("\n\n");
var_export(array('foo', 'bar'));
echo("\n\n");
var_export(array('foo' => 'fooval'));
echo("\n\n");
var_export(array('foo' => 'fooval', 'bar' => 'barval'));
echo("\n\n");
var_export(array('4' => 'zuckval', '4bar' => 'barval'));
echo("\n\n");

echo("** vec **\n");
var_export(vec[]);
echo("\n\n");
var_export(vec['foo']);
echo("\n\n");
var_export(vec['foo', 'bar']);
echo("\n\n");

echo("** dict **\n");
var_export(dict[]);
echo("\n\n");
var_export(dict['foo' => 'fooval']);
echo("\n\n");
var_export(dict['foo' => 'fooval', 'bar' => 'barval']);
echo("\n\n");
var_export(dict['4' => 'zuckval', '4bar' => 'barval']);
echo("\n\n");

echo("** keyset **\n");
var_export(keyset[]);
echo("\n\n");
var_export(keyset['foo']);
echo("\n\n");
var_export(keyset['foo', 'bar']);
echo("\n\n");
}
