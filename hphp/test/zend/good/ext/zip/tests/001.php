<?hh
<<__EntryPoint>> function main(): void {
$get = \HH\global_get('_GET');
parse_str("", inout $get);
\HH\global_set('_GET', $get);

$post = \HH\global_get('_POST');
parse_str("", inout $post);
\HH\global_set('_POST', $post);

// what exactly is this testing?
echo "zip extension is available";
}
