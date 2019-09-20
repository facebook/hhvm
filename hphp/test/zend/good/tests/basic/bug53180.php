<?hh
<<__EntryPoint>>
function main() {
$post = $GLOBALS['_POST'];
parse_str("email=foo&password=bar&submit=Log+on", inout $post);
$GLOBALS['_POST'] = $post;
$_REQUEST = array_merge($_REQUEST, $_POST);

var_dump($_POST);
}
