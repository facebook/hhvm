<?hh
<<__EntryPoint>> function main(): void {
$get = $GLOBALS['_GET'];
parse_str("", inout $get);
$GLOBALS['_GET'] = $get;
$_REQUEST = array_merge($_REQUEST, $_GET);

$post = $GLOBALS['_POST'];
parse_str("", inout $post);
$GLOBALS['_POST'] = $post;
$_REQUEST = array_merge($_REQUEST, $_POST);

// what exactly is this testing?
echo "zip extension is available";
}
