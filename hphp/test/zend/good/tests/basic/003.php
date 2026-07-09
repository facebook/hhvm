<?hh

<<__EntryPoint>>
function main() :mixed{
$get = \HH\global_get('_GET');
parse_str("b=Hello+Again+World&c=Hi+Mom", inout $get);
\HH\global_set('_GET', $get);

$post = \HH\global_get('_POST');
parse_str("a=Hello+World", inout $post);
\HH\global_set('_POST', $post);

error_reporting(0);
echo "post-a=(".\HH\global_get('_POST')['a'].") get-b=(".\HH\global_get('_GET')['b'].") get-c=(".\HH\global_get('_GET')['c'].")";
}
