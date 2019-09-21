<?hh

<<__EntryPoint>>
function main() {
$post = $GLOBALS['_POST'];
parse_str("a=Hello+World&b=Hello+Again+World&c=1", inout $post);
$GLOBALS['_POST'] = $post;
$_REQUEST = array_merge($_REQUEST, $_POST);

error_reporting(0);
echo "{$_POST['a']} {$_POST['b']} {$_POST['c']}";
}
