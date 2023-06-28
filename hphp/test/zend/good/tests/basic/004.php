<?hh

<<__EntryPoint>>
function main() :mixed{
$post = \HH\global_get('_POST');
parse_str("a=Hello+World&b=Hello+Again+World", inout $post);
\HH\global_set('_POST', $post);
$_REQUEST = array_merge($_REQUEST, $_POST);

error_reporting(0);
echo "{$_POST['a']} {$_POST['b']}";
}
