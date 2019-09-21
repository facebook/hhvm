<?hh
<<__EntryPoint>>
function main() {
$post = $GLOBALS['_POST'];
parse_str("a[]=1&a[a]=1&a[b]=3", inout $post);
$GLOBALS['_POST'] = $post;
$_REQUEST = array_merge($_REQUEST, $_POST);

var_dump($_POST['a']);
}
