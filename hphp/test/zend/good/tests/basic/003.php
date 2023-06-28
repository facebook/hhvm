<?hh

<<__EntryPoint>>
function main() :mixed{
$get = \HH\global_get('_GET');
parse_str("b=Hello+Again+World&c=Hi+Mom", inout $get);
\HH\global_set('_GET', $get);
$_REQUEST = array_merge($_REQUEST, $_GET);

$post = \HH\global_get('_POST');
parse_str("a=Hello+World", inout $post);
\HH\global_set('_POST', $post);
$_REQUEST = array_merge($_REQUEST, $_POST);

error_reporting(0);
echo "post-a=({$_POST['a']}) get-b=({$_GET['b']}) get-c=({$_GET['c']})";
}
