<?hh

echo("** array **\n");
var_dump(json_encode(array()));
var_dump(json_encode(array('foo')));
var_dump(json_encode(array('foo', 'bar')));
var_dump(json_encode(array('fookey' => 'fooval', 'barkey' => 'barval')));
var_dump(json_encode(array('fookey' => 'fooval')));

echo("\n** vec **\n");
var_dump(json_encode(vec[]));
var_dump(json_encode(vec['foo']));
var_dump(json_encode(vec['foo', 'bar']));

echo("\n** keyset **\n");
var_dump(json_encode(keyset[]));
var_dump(json_encode(keyset['foo']));
var_dump(json_encode(keyset['foo', 'bar']));

echo("\n** dict **\n");
var_dump(json_encode(dict[]));
var_dump(json_encode(dict['fookey' => 'fooval']));
var_dump(json_encode(dict['fookey' => 'fooval', 'barkey' => 'barval']));
