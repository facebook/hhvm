<?hh

<<__EntryPoint>>
function main() :mixed{
$get = \HH\global_get('_GET');
parse_str("a=1&b=&c=3", inout $get);
\HH\global_set('_GET', $get);
\HH\global_set('_REQUEST', array_merge(\HH\global_get('_REQUEST'), \HH\global_get('_GET')));
echo \HH\global_get('_GET')['a'];
echo \HH\global_get('_GET')['b'];
echo \HH\global_get('_GET')['c'];
}
