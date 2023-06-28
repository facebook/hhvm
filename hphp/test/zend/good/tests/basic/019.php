<?hh
<<__EntryPoint>>
function main() :mixed{
$post = \HH\global_get('_POST');
parse_str("a[]=1&a[]]=3&a[[]=4", inout $post);
\HH\global_set('_POST', $post);
$_REQUEST = array_merge($_REQUEST, $_POST);

var_dump($_POST['a']);
}
