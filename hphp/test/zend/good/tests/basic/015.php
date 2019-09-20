<?hh <<__EntryPoint>> function main(): void {
$post = $_POST;
parse_str("a[]=1&a[0]=5", inout $post);
$_POST = $post;
$_REQUEST = array_merge($_REQUEST, $_POST);

var_dump($_POST['a']);
}
