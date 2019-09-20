<?hh
<<__EntryPoint>>
function main() {
$get = $GLOBALS['_GET'];
parse_str("id=f03_photos&pgurl=http%3A//fifaworldcup.yahoo.com/03/en/photozone/index.html", inout $get);
$GLOBALS['_GET'] = $get;
$_REQUEST = array_merge($_REQUEST, $_GET);

echo $_GET['id'];
echo "\n";
echo $_GET['pgurl'];
echo "\n";
echo $_REQUEST['id'];
echo "\n";
echo $_REQUEST['pgurl'];
}
