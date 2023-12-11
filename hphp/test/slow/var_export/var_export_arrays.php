<?hh


<<__EntryPoint>>
function main_var_export_arrays() :mixed{
echo("** array **\n");
var_export(vec[]);
echo("\n\n");
var_export(vec['foo']);
echo("\n\n");
var_export(vec['foo', 'bar']);
echo("\n\n");
var_export(dict['foo' => 'fooval']);
echo("\n\n");
var_export(dict['foo' => 'fooval', 'bar' => 'barval']);
echo("\n\n");
var_export(dict['4' => 'zuckval', '4bar' => 'barval']);
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
