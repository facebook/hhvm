<?hh
<<__EntryPoint>>
function main() {
$post = \HH\global_get('_POST');
parse_str("a[]=1", inout $post);
\HH\global_set('_POST', $post);
$_REQUEST = array_merge($_REQUEST, $_POST);

var_dump($_POST['a']);
}
