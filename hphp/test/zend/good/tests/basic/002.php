<?hh <<__EntryPoint>> function main(): void {
$post = \HH\global_get('_POST');
parse_str("a=Hello+World", inout $post);
\HH\global_set('_POST',  $post);
\HH\global_set('_REQUEST', array_merge(\HH\global_get('_REQUEST'), \HH\global_get('_POST')));

echo \HH\global_get('_POST')['a'];
}
