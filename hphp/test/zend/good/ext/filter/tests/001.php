<?hh

<<__EntryPoint>>
function main() :mixed{
$get = \HH\global_get('_GET');
parse_str("a=1", inout $get);
\HH\global_set('_GET', $get);
\HH\global_set('_REQUEST', array_merge(\HH\global_get('_REQUEST'), \HH\global_get('_GET')));
echo \HH\global_get('_GET')['a'];
}
