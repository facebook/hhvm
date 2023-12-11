<?hh


<<__EntryPoint>>
function main_json_encode_hh() :mixed{
echo("** array string keys**\n");
var_dump(json_encode(vec[]));
var_dump(json_encode(vec['foo']));
var_dump(json_encode(vec['foo', 'bar']));
var_dump(json_encode(dict['fookey' => 'fooval', 'barkey' => 'barval']));
var_dump(json_encode(dict['fookey' => 'fooval']));

echo("\n** array int keys**\n");
var_dump(json_encode(dict[0 => 'foo', 1 => 'bar']));
var_dump(json_encode(dict[10 => 'foo', 11 => 'bar']));

echo("\n** vec **\n");
var_dump(json_encode(vec[]));
var_dump(json_encode(vec['foo']));
var_dump(json_encode(vec['foo', 'bar']));

echo("\n");
var_dump(json_encode(vec[0, 1]));
var_dump(json_encode(vec[10, 11]));
var_dump(json_encode(vec['10', '11']));

echo("\n** keyset **\n");
var_dump(json_encode(keyset[]));
var_dump(json_encode(keyset['foo']));
var_dump(json_encode(keyset['foo', 'bar']));

echo("\n");
var_dump(json_encode(keyset[0, 1]));
var_dump(json_encode(keyset[10, 11]));
var_dump(json_encode(keyset['0', '1']));
var_dump(json_encode(keyset['10', '11']));

echo("\n** dict **\n");
var_dump(json_encode(dict[]));
var_dump(json_encode(dict['fookey' => 'fooval']));
var_dump(json_encode(dict['fookey' => 'fooval', 'barkey' => 'barval']));

echo("\n");
var_dump(json_encode(dict[0 => 'fooval', 1 => 'barval']));
var_dump(json_encode(dict[10 => 'fooval', 11 => 'barval']));
var_dump(json_encode(dict['0' => 'fooval', '1' => 'barval']));
}
