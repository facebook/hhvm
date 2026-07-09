<?hh
<<__EntryPoint>>
function main() :mixed{
$post = \HH\global_get('_POST');
parse_str("email=foo&password=bar&submit=Log+on", inout $post);
\HH\global_set('_POST', $post);

var_dump(\HH\global_get('_POST'));
}
