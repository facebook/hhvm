<?hh

<<__EntryPoint>>
function main() :mixed{
$get = \HH\global_get('_GET');
parse_str("a=1&b=&c=3", inout $get);
\HH\global_set('_GET', $get);
$_REQUEST = array_merge($_REQUEST, $_GET);

$post = \HH\global_get('_POST');
parse_str("d=4&e=5", inout $post);
\HH\global_set('_POST', $post);
$_REQUEST = array_merge($_REQUEST, $_POST);
echo $_GET['a'];
echo $_GET['b'];
echo $_GET['c'];
echo $_POST['d'];
echo $_POST['e'];
echo "\n";
echo $_REQUEST['a'];
echo $_REQUEST['b'];
echo $_REQUEST['c'];
echo $_REQUEST['d'];
echo $_REQUEST['e'];
}
