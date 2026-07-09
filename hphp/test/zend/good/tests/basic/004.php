<?hh

<<__EntryPoint>>
function main() :mixed{
$post = \HH\global_get('_POST');
parse_str("a=Hello+World&b=Hello+Again+World", inout $post);
\HH\global_set('_POST', $post);

error_reporting(0);
echo \HH\global_get('_POST')['a'].' '.\HH\global_get('_POST')['b'];
}
