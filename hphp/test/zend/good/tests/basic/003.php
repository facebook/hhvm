<?hh

<<__EntryPoint>>
function main() {
$get = $GLOBALS['_GET'];
parse_str("b=Hello+Again+World&c=Hi+Mom", inout $get);
$GLOBALS['_GET'] = $get;
$_REQUEST = array_merge($_REQUEST, $_GET);

$post = $GLOBALS['_POST'];
parse_str("a=Hello+World", inout $post);
$GLOBALS['_POST'] = $post;
$_REQUEST = array_merge($_REQUEST, $_POST);

error_reporting(0);
echo "post-a=({$_POST['a']}) get-b=({$_GET['b']}) get-c=({$_GET['c']})";
}
