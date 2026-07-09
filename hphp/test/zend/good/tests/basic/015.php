<?hh <<__EntryPoint>> function main(): void {
$post = \HH\global_get('_POST');
parse_str("a[]=1&a[0]=5", inout $post);
\HH\global_set('_POST',  $post);

var_dump(\HH\global_get('_POST')['a']);
}
