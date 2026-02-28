<?hh
<<__EntryPoint>> function main(): void {
$get = \HH\global_get('_GET');
parse_str("", inout $get);
\HH\global_set('_GET', $get);
\HH\global_set('_REQUEST', array_merge(\HH\global_get('_REQUEST'), \HH\global_get('_GET')));

$post = \HH\global_get('_POST');
parse_str("", inout $post);
\HH\global_set('_POST', $post);
\HH\global_set('_REQUEST', array_merge(\HH\global_get('_REQUEST'), \HH\global_get('_POST')));

// what exactly is this testing?
echo "zip extension is available";
}
