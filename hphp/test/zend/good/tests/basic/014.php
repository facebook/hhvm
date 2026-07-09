<?hh

<<__EntryPoint>>
function main() :mixed{
$post = \HH\global_get('_POST');
parse_str("a[]=1&a[]=1", inout $post);
\HH\global_set('_POST', $post);

var_dump(\HH\global_get('_POST')['a']);
}
