<?hh

<<__EntryPoint>>
function main() :mixed{
$get = \HH\global_get('_GET');
parse_str("a=1&b=&c=3", inout $get);
\HH\global_set('_GET', $get);

$post = \HH\global_get('_POST');
parse_str("d=4&e=5", inout $post);
\HH\global_set('_POST', $post);
echo \HH\global_get('_GET')['a'];
echo \HH\global_get('_GET')['b'];
echo \HH\global_get('_GET')['c'];
echo \HH\global_get('_POST')['d'];
echo \HH\global_get('_POST')['e'];
echo "\n";
echo \HH\global_get('_GET')['a'];
echo \HH\global_get('_GET')['b'];
echo \HH\global_get('_GET')['c'];
echo \HH\global_get('_POST')['d'];
echo \HH\global_get('_POST')['e'];
}
